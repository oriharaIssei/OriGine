#include "SmoothingEffect.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"

// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void SmoothingEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();

    boxFilterSize_.CreateBuffer(Engine::getInstance()->getDxDevice()->device_);

    setBoxFilterSize(5.f, 5.f);
}

void SmoothingEffect::Update() {
    Render();
}

void SmoothingEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_ = nullptr;
}

void SmoothingEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
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

    pso_ = shaderManager->CreatePso("SmoothingEffect", shaderInfo, Engine::getInstance()->getDxDevice()->device_);
}

void SmoothingEffect::Render() {
    auto& commandList = dxCommand_->getCommandList();
    auto sceneView    = this->getScene()->getSceneView();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /// ================================================
    /// Data
    /// ================================================
    boxFilterSize_.SetForRootParameter(commandList, 0);

    /// ================================================
    /// Viewport の設定
    /// ================================================
    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    commandList->SetGraphicsRootDescriptorTable(1, sceneView->getBackBufferSrvHandle());

    commandList->DrawInstanced(6, 1, 0, 0);
}
