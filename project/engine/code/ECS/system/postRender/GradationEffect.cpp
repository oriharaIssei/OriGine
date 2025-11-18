#include "GradationEffect.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"

// component
#include "component/effect/post/GradationTextureComponent.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

GradationEffect::GradationEffect() : BasePostRenderingSystem() {}
GradationEffect::~GradationEffect() {}

void GradationEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void GradationEffect::Finalize() {
    if (dxCommand_) {
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void GradationEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Gradation.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Gradation.PS";

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

    pso_ = shaderManager->CreatePso("GradationEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

void GradationEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    renderTarget_->PreDraw();

    /// ================================================
    /// pso Set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void GradationEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    for (auto& data : activeRenderingData_) {
        // レンダリング開始処理
        RenderStart();

        auto& paramBuff   = data.effectParam->GetParamBuff();
        auto& uvTransBuff = data.effectParam->GetMaterialBuff();

        commandList->SetGraphicsRootDescriptorTable(0, data.srvHandle);
        commandList->SetGraphicsRootDescriptorTable(1, renderTarget_->GetBackBufferSrvHandle());
        paramBuff.SetForRootParameter(commandList, 2);
        uvTransBuff.SetForRootParameter(commandList, 3);

        // 描画
        commandList->DrawInstanced(6, 1, 0, 0);

        // レンダリング終了処理
        RenderEnd();
    }
    // 描画データのクリア
    activeRenderingData_.clear();
}

void GradationEffect::RenderEnd() {
    renderTarget_->PostDraw();
}

void GradationEffect::DispatchComponent(Entity* _entity) {
    auto effectParams = GetComponents<GradationTextureComponent>(_entity);

    if (!effectParams) {
        return; // コンポーネントがない場合は何もしない
    }

    for (auto& param : *effectParams) {
        if (!param.IsActive()) {
            continue;
        }
        RenderingData data{};

        D3D12_GPU_DESCRIPTOR_HANDLE texHandle = TextureManager::GetDescriptorGpuHandle(param.GetTextureIndex());

        int32_t materialIndex = param.GetMaterialIndex();
        auto& uvTransBuff     = param.GetMaterialBuff();
        if (materialIndex >= 0) {
            Material* material = GetComponent<Material>(_entity, materialIndex);
            material->UpdateUvMatrix();

            uvTransBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                texHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
            }
        }

        param.GetParamBuff().ConvertToBuffer();

        data.effectParam = &param;
        data.srvHandle   = texHandle;
        activeRenderingData_.emplace_back(data);
    }
}

bool GradationEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty();
}
