#include "RenderTexture.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ResourceStateTracker.h"
#include "directX12/ShaderManager.h"

/// externals
#include "logger/Logger.h"

/// math
#include "Vector2.h"

/// util
#include "util/ConvertString.h"

PipelineStateObj* RenderTexture::pso_;

void RenderTexture::Awake() {
    ShaderManager* shaderManager = ShaderManager::getInstance();

    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("FullScreen.PS", shaderDirectory, L"ps_6_0");

    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "FullScreen.PS";

    ///================================================
    /// Sampler の設定
    ///================================================
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MinLOD           = 0;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(sampler);

    ///================================================
    /// RootParameter の設定
    ///================================================
    // Texture だけ
    D3D12_ROOT_PARAMETER rootParameter        = {};
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister     = 0;
    descriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex      = shaderInfo.pushBackRootParameter(rootParameter);
    shaderInfo.setDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

    ///================================================
    /// InputElement の設定
    ///================================================

    // 特に使わない

    ///================================================
    /// depthStencil の設定
    ///================================================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
    shaderInfo.setDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("FullScreen", shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
}

void RenderTexture::Initialize(int32_t _bufferCount, const Vec2f& textureSize, DXGI_FORMAT _format, const Vec4f& _clearColor) {
    format_      = _format;
    bufferCount_ = _bufferCount;
    renderTargets_.resize(bufferCount_);

    textureSize_ = textureSize;
    clearColor_  = _clearColor;

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    ///===========================================================================
    /// RenderTexture Resource の作成
    ///===========================================================================
    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::getInstance()->getDxDevice()->getDevice();

    for (auto& renderTarget : renderTargets_) {
        renderTarget.resource_.CreateRenderTextureResource(
            device,
            static_cast<uint32_t>(textureSize_[X]),
            static_cast<uint32_t>(textureSize_[Y]),
            format_,
            clearColor_);

        /// ------------------------------------------------------------------
        ///  RTV の作成
        /// ------------------------------------------------------------------
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format        = format_;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTarget.rtv_     = Engine::getInstance()->getRtvHeap()->CreateDescriptor(rtvDesc, &renderTarget.resource_);

        /// ------------------------------------------------------------------
        ///  SRV の作成
        ///------------------------------------------------------------------
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                  = format_;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels     = 1;

        renderTarget.srv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor(srvDesc, &renderTarget.resource_);

        /// ------------------------------------------------------------------
        ///  ResourceStateTracker の登録
        /// ------------------------------------------------------------------
        ResourceStateTracker::RegisterResource(renderTarget.resource_.getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    std::wstring wName = ConvertString(textureName_);
    int32_t index      = 0;
    for (auto& renderTarget : renderTargets_) {
        renderTarget.resource_.setName(wName + std::to_wstring(index));
        ++index;
    }
}

void RenderTexture::Resize(const Vec2f& textureSize) {
    if (Vec2f(textureSize_ - textureSize).lengthSq() < 0.1f) {
        return;
    }

    frontBufferIndex_ = 0;
    backBufferIndex_  = 0;
    textureSize_      = textureSize;

    // 古いリソースを解放
    for (auto& target : renderTargets_) {
        target.resource_.Finalize();
        Engine::getInstance()->getRtvHeap()->ReleaseDescriptor(target.rtv_);
        Engine::getInstance()->getSrvHeap()->ReleaseDescriptor(target.srv_);
    }

    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::getInstance()->getDxDevice()->getDevice();

    std::wstring wName = ConvertString(textureName_);
    // 新しいリソースを作成
    for (int i = 0; i < bufferCount_; ++i) {
        renderTargets_[i].resource_.CreateRenderTextureResource(
            device,
            static_cast<uint32_t>(textureSize_[X]),
            static_cast<uint32_t>(textureSize_[Y]),
            format_,
            clearColor_);

        /// ------------------------------------------------------------------
        ///  RTV の作成
        /// ------------------------------------------------------------------
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format         = format_;
        rtvDesc.ViewDimension  = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargets_[i].rtv_ = Engine::getInstance()->getRtvHeap()->CreateDescriptor(rtvDesc, &renderTargets_[i].resource_);
        /// ------------------------------------------------------------------
        ///  SRV の作成
        ///------------------------------------------------------------------
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                  = format_;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels     = 1;

        renderTargets_[i].srv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor(srvDesc, &renderTargets_[i].resource_);

        /// ------------------------------------------------------------------
        ///  ResourceStateTracker の登録
        /// ------------------------------------------------------------------
        ResourceStateTracker::RegisterResource(renderTargets_[i].resource_.getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        renderTargets_[i].resource_.setName(wName + std::to_wstring(i));
    }
}

void RenderTexture::Finalize() {
    auto srvHeap = Engine::getInstance()->getSrvHeap();
    auto rtvHeap = Engine::getInstance()->getRtvHeap();
    for (auto& renderTarget : renderTargets_) {
        renderTarget.resource_.Finalize();

        rtvHeap->ReleaseDescriptor(renderTarget.rtv_);
        srvHeap->ReleaseDescriptor(renderTarget.srv_);

        renderTarget.rtv_.reset();
        renderTarget.srv_.reset();
    }

    dxCommand_->Finalize();
}

void RenderTexture::PreDraw(DxDsvDescriptor* _dsv) {

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->getCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = getFrontBufferRtvHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsv->getCpuHandle();

    ///=========================================
    //	TransitionBarrier の 設定
    ///=========================================
    dxCommand_->ResourceBarrier(
        getFrontBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    ///=========================================
    //	Clear RTV
    ///=========================================
    float clearColor[4] = {
        clearColor_[X],
        clearColor_[Y],
        clearColor_[Z],
        clearColor_[W]};
    commandList->ClearRenderTargetView(
        rtvHandle, clearColor_.v, 0, nullptr);
    ///=========================================
    //	Clear DSV
    ///=========================================
    commandList->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    ///=========================================
    //	ビューポート の 設定
    ///=========================================
    D3D12_VIEWPORT viewPort{};
    viewPort.Width    = textureSize_[X];
    viewPort.Height   = textureSize_[Y];
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewPort);
    ///=========================================
    //	シザーレクト の 設定
    ///=========================================
    D3D12_RECT scissorRect{};
    scissorRect.left   = 0;
    scissorRect.right  = static_cast<LONG>(textureSize_[X]);
    scissorRect.top    = 0;
    scissorRect.bottom = static_cast<LONG>(textureSize_[Y]);

    commandList->RSSetScissorRects(1, &scissorRect);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RenderTexture::PostDraw() {
    HRESULT hr;
    DxFence* fence = Engine::getInstance()->getDxFence();

    ///===============================================================
    ///	バリアの更新(描画->表示状態)
    ///===============================================================
    dxCommand_->ResourceBarrier(
        getFrontBuffer(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    ///===============================================================

    // コマンドの受付終了 -----------------------------------
    hr = dxCommand_->Close();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(hr));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand_->ExecuteCommand();
    ///===============================================================

    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    fence->Signal(dxCommand_->getCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================

    // log
    LOG_DX12();

    ///===============================================================
    /// bufferIndex の更新
    ///===============================================================
    if (bufferCount_ > 1) {
        backBufferIndex_  = frontBufferIndex_;
        frontBufferIndex_ = (frontBufferIndex_ + 1) % bufferCount_;
    }
}

void RenderTexture::DrawTexture() {
    auto& commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    commandList->SetGraphicsRootDescriptorTable(
        0,
        getBackBufferSrvHandle());

    commandList->DrawInstanced(6, 1, 0, 0);
}

void RenderTexture::setDxCommand(std::unique_ptr<DxCommand>&& _dxCommand) {
    dxCommand_->Finalize();
    dxCommand_.reset();

    dxCommand_ = std::move(_dxCommand);
}

void RenderTexture::setTextureName(const std::string& _name) {
    textureName_       = _name;
    std::wstring wName = ConvertString(_name);
    int32_t index      = 0;
    for (auto& renderTarget : renderTargets_) {
        renderTarget.resource_.setName(wName + std::to_wstring(index));
        ++index;
    }
}
