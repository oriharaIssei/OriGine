#include "SmoothingEffect.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"

// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

SmoothingEffect::SmoothingEffect() : BasePostRenderingSystem() {}
SmoothingEffect::~SmoothingEffect() {}

void SmoothingEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

void SmoothingEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_ = nullptr;
}

void SmoothingEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Smoothing.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Smoothing.PS";

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
    // boxFilterSize
    D3D12_ROOT_PARAMETER boxFilterSizeParam{};
    // Transform ... 0
    boxFilterSizeParam.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    boxFilterSizeParam.ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    boxFilterSizeParam.Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(boxFilterSizeParam);

    // Texture
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
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

    ///================================================
    /// InputElement の設定
    ///================================================

    // 特に使わない

    ///================================================
    /// depthStencil の設定
    ///================================================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
    shaderInfo.SetDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("SmoothingEffect", shaderInfo, OriGine::Engine::GetInstance()->GetDxDevice()->device_);
}

void SmoothingEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    renderTarget_->PreDraw();

    /// ================================================
    /// pso Set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {OriGine::Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void SmoothingEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    for (auto& param : activeParams_) {
        // レンダー開始
        RenderStart();

        // 描画
        param->boxFilterSize_.ConvertToBuffer();
        param->boxFilterSize_.SetForRootParameter(commandList, 0);

        commandList->SetGraphicsRootDescriptorTable(1, renderTarget_->GetBackBufferSrvHandle());
        commandList->DrawInstanced(6, 1, 0, 0);

        // レンダー終了
        RenderEnd();
    }

    // 処理が終わったらクリア
    activeParams_.clear();
}

void SmoothingEffect::RenderEnd() {
    renderTarget_->PostDraw();
}

void SmoothingEffect::DispatchComponent(Entity* _entity) {
    auto* params = GetComponents<SmoothingEffectParam>(_entity);
    if (!params) {
        return;
    }

    for (auto& param : *params) {
        if (!param.isActive_) {
            continue;
        }
        activeParams_.push_back(&param);
    }
}

bool SmoothingEffect::ShouldSkipPostRender() const {
    return activeParams_.empty();
}
