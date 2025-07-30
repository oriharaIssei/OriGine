#include "RandomEffect.h"

/// engine
#include "Engine.h"

// component
#include "component/ComponentArray.h"
#include "component/effect/post/RandomEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void RandomEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePSO();
}

void RandomEffect::Update() {
    auto* sceneView = getScene()->getSceneView();

    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    sceneView->PreDraw();
    sceneView->DrawTexture();
    RenderStart();

    for (auto& id : entityIDs_) {
        auto* entity = getEntity(id);
        UpdateEntity(entity);
    }

    Render();

    sceneView->PostDraw();
}

void RandomEffect::UpdateEntity(GameEntity* _entity) {
    RandomEffectParam paramA;

    int32_t compSize = getComponentArray<RandomEffectParam>()->getComponentSize(_entity);
    if (compSize <= 0) {
        return;
    }

    const float deltaTime = Engine::getInstance()->getDeltaTime();
    auto& commandList     = dxCommand_->getCommandList();

    for (int32_t i = 0; i < compSize; ++i) {
        auto* param = getComponent<RandomEffectParam>(_entity, i);
        if (param == nullptr) {
            continue;
        }
        // パラメータの更新
        float time = param->getCurrentTime();
        time += deltaTime;
        if (time > param->getMaxTime()) {
            time = 0.f;
        }
        param->setCurrentTime(time);

        ChangeBlendMode(param->getBlendMode());

        // constant buffer の更新
        param->getConstantBuffer().ConvertToBuffer();
        param->getConstantBuffer().SetForRootParameter(commandList, 0);
    }
}

void RandomEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_.clear();
}

void RandomEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Random.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Random.PS";

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
    D3D12_ROOT_PARAMETER rootParameter[1] = {};

    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

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

    for (size_t i = 0; i < static_cast<size_t>(BlendMode::Count); ++i) {
        BlendMode blendMode   = static_cast<BlendMode>(i);
        shaderInfo.blendMode_ = blendMode;
        pso_[blendMode]       = shaderManager->CreatePso("RandomEffect_" + blendModeStr[i], shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
    }
}

void RandomEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void RandomEffect::Render() {
    auto& commandList = dxCommand_->getCommandList();

    commandList->DrawInstanced(6, 1, 0, 0);
}

void RandomEffect::ChangeBlendMode(BlendMode mode) {
    if (currentBlend_ == mode) {
        return;
    }
    currentBlend_ = mode;

    auto& commandList = dxCommand_->getCommandList();
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
}
