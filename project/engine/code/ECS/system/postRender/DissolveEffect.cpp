#include "DissolveEffect.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"

// component
#include "component/effect/post/DissolveEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

DissolveEffect::DissolveEffect() : BasePostRenderingSystem() {}
DissolveEffect::~DissolveEffect() {}

void DissolveEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

void DissolveEffect::Finalize() {
    BasePostRenderingSystem::Finalize();
    pso_ = nullptr;
}

void DissolveEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Dissolve.PS", kShaderDirectory, L"ps_6_0");
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
    D3D12_ROOT_PARAMETER rootParameter[4]    = {};
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
    shaderInfo.SetDescriptorRange2Parameter(sceneViewRange, 1, sceneViewParamIdx);

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
    shaderInfo.SetDescriptorRange2Parameter(effectTexRange, 1, effectTexParamIdx);

    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[3]);

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

    pso_ = shaderManager->CreatePso("DissolveEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

bool DissolveEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty();
}

void DissolveEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    /// target の設定
    renderTarget_->PreDraw();

    /// pso Set
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void DissolveEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    for (const auto& renderingData : activeRenderingData_) {
        /// 描画 開始
        RenderStart();

        commandList->SetGraphicsRootDescriptorTable(1,
            renderingData.srvHandle);

        auto& dissParam = renderingData.dissolveParam;
        dissParam->GetDissolveBuffer().ConvertToBuffer();
        dissParam->GetDissolveBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 2);
        dissParam->GetMaterialBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 3);
        /// 描画 呼び出し
        RenderCall(renderTarget_->GetBackBufferSrvHandle());

        // 描画 終了
        RenderEnd();
    }

    /// アクティブなレンダリングデータのクリア
    activeRenderingData_.clear();
}

void DissolveEffect::RenderEnd() {
    // 描画 終了
    renderTarget_->PostDraw();
}

void DissolveEffect::DispatchComponent(EntityHandle _handle) {
    auto& effectParams = GetComponents<DissolveEffectParam>(_handle);

    if (effectParams.empty()) {
        return; // コンポーネントがない場合は何もしない
    }

    for (auto& param : effectParams) {
        if (!param.IsActive()) {
            continue;
        }

        DissolveEffect::RenderingData renderingData{};
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetDescriptorGpuHandle(param.GetTextureIndex());

        int32_t materialIndex = param.GetMaterialIndex();
        auto& materialBuff    = param.GetMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = GetComponent<Material>(_handle, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                srvHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
            }
        }

        renderingData.srvHandle     = srvHandle;
        renderingData.dissolveParam = &param;
        activeRenderingData_.push_back(renderingData);
    }
}

void DissolveEffect::RenderCall(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle) {
    auto& commandList = dxCommand_->GetCommandList();

    /// ================================================
    /// Viewport の設定
    /// ================================================

    commandList->SetGraphicsRootDescriptorTable(0, _viewHandle);

    commandList->DrawInstanced(6, 1, 0, 0);
}
