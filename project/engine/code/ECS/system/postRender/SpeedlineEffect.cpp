#include "SpeedlineEffect.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"
#include "texture/TextureManager.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void SpeedlineEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void SpeedlineEffect::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    eraseDeadEntity();

    // アクティブなパラメータを収集
    activeParams_.clear();
    for (auto& id : entityIDs_) {
        auto* entity = getEntity(id);
        auto* params = getComponents<SpeedlineEffectParam>(entity);
        if (!params) {
            continue;
        }
        for (auto& param : *params) {
            if (param.isActive()) {
                activeParams_.push_back(&param);
            }
        }
    }
    if (activeParams_.empty()) {
        return;
    }

    RenderStart();

    for (auto& param : activeParams_) {
        Render(param);
    }

    RenderEnd();
}

void SpeedlineEffect::UpdateEntity(Entity* _entity) {
    auto* speedlineParam = getComponent<SpeedlineEffectParam>(_entity);
    speedlineParam->getBuffer().ConvertToBuffer();
    speedlineParam->getBuffer().SetForRootParameter(dxCommand_->getCommandList(), 1);
}

void SpeedlineEffect::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void SpeedlineEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Speedline.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Speedline.PS";

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
    D3D12_ROOT_PARAMETER rootParameter[3]             = {};
    D3D12_DESCRIPTOR_RANGE sceneTexDescriptorRange[1] = {};
    sceneTexDescriptorRange[0].BaseShaderRegister     = 0;
    sceneTexDescriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    sceneTexDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    sceneTexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.setDescriptorRange2Parameter(sceneTexDescriptorRange, 1, rootParameterIndex);

    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[1]);

    D3D12_DESCRIPTOR_RANGE radialTexDescriptorRange[1] = {};
    radialTexDescriptorRange[0].BaseShaderRegister     = 1;
    radialTexDescriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    radialTexDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    radialTexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameterIndex                = shaderInfo.pushBackRootParameter(rootParameter[2]);
    shaderInfo.setDescriptorRange2Parameter(radialTexDescriptorRange, 1, rootParameterIndex);

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

    pso_ = shaderManager->CreatePso("SpeedlineEffect", shaderInfo, Engine::getInstance()->getDxDevice()->device_);
}

void SpeedlineEffect::RenderStart() {
    auto commandList = dxCommand_->getCommandList();

    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    auto sceneView = this->getScene()->getSceneView();
    sceneView->PreDraw();
}

void SpeedlineEffect::Render(SpeedlineEffectParam* _param) {
    auto& commandList = dxCommand_->getCommandList();
    auto* sceneView   = this->getScene()->getSceneView();

    _param->getBuffer().ConvertToBuffer();
    _param->getBuffer().SetForRootParameter(dxCommand_->getCommandList(), 1);

    commandList->SetGraphicsRootDescriptorTable(
        2,
        TextureManager::getDescriptorGpuHandle(_param->getRadialTextureIndex()));

    /// ================================================
    /// Viewport の設定
    /// ================================================
    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    commandList->SetGraphicsRootDescriptorTable(0, sceneView->getBackBufferSrvHandle());

    commandList->DrawInstanced(6, 1, 0, 0);
}

void SpeedlineEffect::RenderEnd() {
    auto sceneView = this->getScene()->getSceneView();
    sceneView->PostDraw();
}
