#include "RadialBlurEffect.h"

/// engine
#include "Engine.h"

// component
#include "component/effect/post/RadialBlurParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

RadialBlurEffect::RadialBlurEffect() : BasePostRenderingSystem() {}
RadialBlurEffect::~RadialBlurEffect() {}

void RadialBlurEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

void RadialBlurEffect::Finalize() {
    BasePostRenderingSystem::Finalize();
    pso_ = nullptr;
}

void RadialBlurEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("RadialBlur.PS", kShaderDirectory, L"ps_6_0");
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
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

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
    shaderInfo.SetDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("RadialBlurEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

void RadialBlurEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    // 描画設定

    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    commandList->SetGraphicsRootDescriptorTable(0, renderTarget_->GetBackBufferSrvHandle());
}

void RadialBlurEffect::Rendering() {
    for (auto& param : activeRadialBlurParams_) {

        RenderStart();

        param->GetConstantBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 1);

        RenderEnd();
    }

    activeRadialBlurParams_.clear();
}

void RadialBlurEffect::RenderEnd() {
    renderTarget_->PostDraw();
}

void RadialBlurEffect::DispatchComponent(Entity* _entity) {
    auto components = GetComponents<RadialBlurParam>(_entity);
    if (!components) {
        return;
    }

    // アクティブなコンポーネントだけ登録する
    for (auto& component : *components) {
        if (!component.IsActive()) {
            continue;
        }
        component.GetConstantBuffer().ConvertToBuffer();
        activeRadialBlurParams_.emplace_back(&component);
    }
}

bool RadialBlurEffect::ShouldSkipPostRender() const {
    return activeRadialBlurParams_.empty();
}
