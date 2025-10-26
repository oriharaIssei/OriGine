#include "RadialBlurEffect.h"

/// engine
#include "Engine.h"

// component
#include "component/effect/post/RadialBlurParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void RadialBlurEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void RadialBlurEffect::Update() {
    auto* sceneView = getScene()->getSceneView();

    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    activeRadialBlurParams_.clear();

    for (auto& id : entityIDs_) {
        auto* entity = getEntity(id);

        int32_t size = getComponentArray<RadialBlurParam>()->getComponentSize(entity);
        for (int32_t i = 0; i < size; ++i) {
            auto* radialBlurParam = getComponent<RadialBlurParam>(entity, i);
            if (!radialBlurParam || !radialBlurParam->isActive()) {
                continue;
            }
            activeRadialBlurParams_.emplace_back(radialBlurParam);
        }
    }

    // アクティブなパラメータがない場合は何もしない
    if (activeRadialBlurParams_.empty()) {
        return;
    }

    for (auto& activeParam : activeRadialBlurParams_) {
        sceneView->PreDraw();
        RenderStart();

        activeParam->getConstantBuffer().ConvertToBuffer();
        activeParam->getConstantBuffer().SetForRootParameter(dxCommand_->getCommandList(), 1);
        Render();

        sceneView->PostDraw();
    }
}

void RadialBlurEffect::UpdateEntity(Entity* _entity) {
    int32_t size = getComponentArray<RadialBlurParam>()->getComponentSize(_entity);
    for (int32_t i = 0; i < size; ++i) {
        auto* radialBlurParam = getComponent<RadialBlurParam>(_entity, i);
        if (!radialBlurParam->isActive()) {
            return;
        }
        radialBlurParam->getConstantBuffer().ConvertToBuffer();
        radialBlurParam->getConstantBuffer().SetForRootParameter(dxCommand_->getCommandList(), 1);
    }
}

void RadialBlurEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_ = nullptr;
}

void RadialBlurEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("RadialBlur.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "RadialBlur.PS";

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

    pso_ = shaderManager->CreatePso("RadialBlurEffect", shaderInfo, Engine::getInstance()->getDxDevice()->device_);
}

void RadialBlurEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RadialBlurEffect::Render() {
    auto& commandList = dxCommand_->getCommandList();
    auto* sceneView   = getScene()->getSceneView();

    /// ================================================
    /// Viewport の設定
    /// ================================================
    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    commandList->SetGraphicsRootDescriptorTable(0, sceneView->getBackBufferSrvHandle());

    commandList->DrawInstanced(6, 1, 0, 0);
}
