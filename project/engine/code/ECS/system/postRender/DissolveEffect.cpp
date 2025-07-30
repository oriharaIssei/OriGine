#include "DissolveEffect.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"

// component
#include "component/effect/post/DissolveEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void DissolveEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void DissolveEffect::Update() {
    auto* sceneView = this->getScene()->getSceneView();

    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    // 実行する必要があるかチェック
    bool allIsUnactive   = true;
    auto* componentArray = getComponentArray<DissolveEffectParam>();
    if (!componentArray) {
        return;
    }
    for (auto& compVec : *componentArray->getAllComponents()) {
        if (compVec.empty()) {
            continue;
        }
        for (auto& comp : compVec) {
            if (comp.isActive()) {
                allIsUnactive = false;
                break; // 1つでもアクティブなコンポーネントがあればループを抜ける
            }
        }
        if (!allIsUnactive) {
            break; // 全てのコンポーネントが非アクティブなら何もしない
        }
    }

    if (allIsUnactive) {
        return; // 全てのコンポーネントが非アクティブなら何もしない
    }

    RenderStart();

    sceneView->PreDraw();
    for (auto& id : entityIDs_) {
        auto* entity = getEntity(id);
        UpdateEntity(entity);
    }
    Render();

    sceneView->PostDraw();
}

void DissolveEffect::UpdateEntity(GameEntity* _entity) {
    int32_t compSize = getComponentArray<DissolveEffectParam>()->getComponentSize(_entity);

    if (compSize <= 0) {
        return; // コンポーネントがない場合は何もしない
    }
    auto& commandList = dxCommand_->getCommandList();

    for (int32_t i = 0; i < compSize; i++) {
        auto* dissolveEffectParam = getComponent<DissolveEffectParam>(_entity,i);
        if (!dissolveEffectParam->isActive()) {
            continue;
        }
        commandList->SetGraphicsRootDescriptorTable(1,
            TextureManager::getDescriptorGpuHandle(dissolveEffectParam->getTextureIndex()));

        dissolveEffectParam->getDissolveBuffer().ConvertToBuffer();
        dissolveEffectParam->getDissolveBuffer().SetForRootParameter(dxCommand_->getCommandList(), 2);
    }
}

void DissolveEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_ = nullptr;
}

void DissolveEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Dissolve.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Dissolve.PS";

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
    D3D12_ROOT_PARAMETER rootParameter[3]    = {};
    D3D12_DESCRIPTOR_RANGE sceneViewRange[1] = {};
    sceneViewRange[0].BaseShaderRegister     = 0;
    sceneViewRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    sceneViewRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    sceneViewRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t sceneViewParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.setDescriptorRange2Parameter(sceneViewRange, 1, sceneViewParamIdx);

    D3D12_DESCRIPTOR_RANGE effectTexRange[1] = {};
    effectTexRange[0].BaseShaderRegister     = 1;
    effectTexRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    effectTexRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    effectTexRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t effectTexParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.setDescriptorRange2Parameter(effectTexRange, 1, effectTexParamIdx);

    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[2]);

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

    pso_ = shaderManager->CreatePso("DissolveEffect", shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
}

void DissolveEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void DissolveEffect::Render() {
    auto& commandList = dxCommand_->getCommandList();
    auto* sceneView   = getScene()->getSceneView();

    /// ================================================
    /// Viewport の設定
    /// ================================================

    commandList->SetGraphicsRootDescriptorTable(0, sceneView->getBackBufferSrvHandle());

    commandList->DrawInstanced(6, 1, 0, 0);
}
