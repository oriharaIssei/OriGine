#include "RandomEffect.h"

/// engine
#include "Engine.h"

// component
#include "component/ComponentArray.h"
#include "component/effect/post/RandomEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

RandomEffect::RandomEffect() : BasePostRenderingSystem() {}
RandomEffect::~RandomEffect() {}

void RandomEffect::Initialize() {
    constexpr size_t kParamContainerReserveSize = 16;
    BasePostRenderingSystem::Initialize();

    // container の初期化
    activeParams_ = {};
    activeParams_.reserve(kParamContainerReserveSize);
}

void RandomEffect::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
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
        psoByBlendMode_[i]    = shaderManager->CreatePso("RandomEffect_" + blendModeStr[i], shaderInfo, Engine::getInstance()->getDxDevice()->device_);
    }
}

void RandomEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    // RenderTargetのセット
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    /// ================================================
    /// pso set
    /// ================================================
    int32_t blendIndex = static_cast<int32_t>(currentBlend_);
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void RandomEffect::Rendering() {
    auto& commandList = dxCommand_->getCommandList();

    for (auto* param : activeParams_) {
        // ブレンドモードの変更
        currentBlend_ = param->getBlendMode();
        // レンダリング開始処理
        RenderStart();

        // constant buffer の更新
        param->getConstantBuffer().ConvertToBuffer();
        param->getConstantBuffer().SetForRootParameter(commandList, 0);
        // 描画
        commandList->DrawInstanced(6, 1, 0, 0);

        // レンダリング終了処理
        RenderEnd();
    }

    activeParams_.clear();
}

void RandomEffect::RenderEnd() {
    renderTarget_->PostDraw();
}

void RandomEffect::DispatchComponent(Entity* _entity) {
    // activeなComponentをBlendModeごとに振り分ける
    auto components = getComponents<RandomEffectParam>(_entity);

    if (!components) {
        return;
    }

    for (auto& comp : *components) {
        if (!comp.isActive()) {
            continue;
        }

        activeParams_.emplace_back(&comp);
    }
}

bool RandomEffect::ShouldSkipPostRender() const {
    return activeParams_.empty();
}
