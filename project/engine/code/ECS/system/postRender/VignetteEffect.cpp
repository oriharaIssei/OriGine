#include "VignetteEffect.h"

/// engine

#include "Engine.h"
#include "scene/SceneManager.h"

// component
#include "component/effect/post/VignetteParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

VignetteEffect::VignetteEffect() : BasePostRenderingSystem() {}
VignetteEffect::~VignetteEffect() {}

void VignetteEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void VignetteEffect::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void VignetteEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Vignette.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Vignette.PS";

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
    D3D12_ROOT_PARAMETER rootParameter[2]     = {};
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister     = 0;
    descriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.setDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[1]);

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

    pso_ = shaderManager->CreatePso("VignetteEffect", shaderInfo, Engine::getInstance()->getDxDevice()->device_);
}

void VignetteEffect::RenderStart() {
    auto commandList = dxCommand_->getCommandList();

    renderTarget_->PreDraw();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void VignetteEffect::Rendering() {
    auto& commandList = dxCommand_->getCommandList();

    for (auto& param : activeParams_) {
        // 描画開始
        RenderStart();

        // 描画処理
        commandList->SetGraphicsRootDescriptorTable(0, renderTarget_->getBackBufferSrvHandle());
        param->getVignetteBuffer().SetForRootParameter(commandList, 1);
        commandList->DrawInstanced(6, 1, 0, 0);

        // 描画終了
        RenderEnd();
    }

    // 使用したパラメータをクリア
    activeParams_.clear();
}

void VignetteEffect::RenderEnd() {
    renderTarget_->PostDraw();
}

void VignetteEffect::DispatchComponent(Entity* _entity) {
    auto* vignetteParams = getComponents<VignetteParam>(_entity);

    if (!vignetteParams) {
        return;
    }

    for (auto& param : *vignetteParams) {
        param.getVignetteBuffer().ConvertToBuffer();
        activeParams_.push_back(&param);
    }
}

bool VignetteEffect::ShouldSkipPostRender() const {
    return activeParams_.empty();
}
