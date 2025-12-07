#include "SkyboxRender.h"

/// engine
#include "camera/CameraManager.h"
#include "Engine.h"
// dx12
#include "directX12/DxDevice.h"
// ecs
#include "component/renderer/SkyboxRenderer.h"

SkyboxRender::SkyboxRender() : BaseRenderSystem() {}
SkyboxRender::~SkyboxRender() {}

void SkyboxRender::Initialize() {
    BaseRenderSystem::Initialize();
    rendererByBlendMode_.fill(std::vector<SkyboxRenderer*>());
}

void SkyboxRender::Finalize() {
    dxCommand_->Finalize();
}

void SkyboxRender::StartRender() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

void SkyboxRender::DispatchRenderer(Entity* _entity) {
    std::vector<SkyboxRenderer>* renderers = GetComponents<SkyboxRenderer>(_entity);
    // nullptr なら これ以上存在しないとして終了
    if (!renderers) {
        return;
    }

    for (auto& renderer : *renderers) {
        // 描画フラグが立っていないならスキップ
        if (!renderer.IsRender()) {
            continue;
        }
        BlendMode rendererBlend = renderer.GetCurrentBlend();
        rendererByBlendMode_[static_cast<int32_t>(rendererBlend)].push_back(&renderer);
    }
}

void SkyboxRender::RenderingBy(BlendMode _blendMode, bool /*_isCulling*/) {
    auto& commandList = dxCommand_->GetCommandList();
    auto& renderers  = rendererByBlendMode_[static_cast<int32_t>(_blendMode)];
    if (renderers.empty()) {
        return;
    }

    // blendmode の設定
    int32_t blendModeIndex = static_cast<int32_t>(_blendMode);
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendModeIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[blendModeIndex]->pipelineState.Get());

    // 描画
    for (auto renderer : renderers) {
        auto& mesh = renderer->GetMeshGroup()->front();
        // ============================= テクスチャの設定 ============================= //

        commandList->SetGraphicsRootDescriptorTable(
            2,
            TextureManager::GetDescriptorGpuHandle(renderer->GetTextureIndex()));

        // ============================= Viewのセット ============================= //
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
        commandList->IASetIndexBuffer(&mesh.GetIBView());

        // ============================= Transformのセット ============================= //
        IConstantBuffer<Transform>& meshTransform = renderer->GetTransformBuff();
        const CameraTransform& cameraTransform    = CameraManager::GetInstance()->GetTransform();
        meshTransform->translate                  = cameraTransform.translate;
        meshTransform->UpdateMatrix();
        const Matrix4x4& viewMat = cameraTransform.viewMat;
        const Matrix4x4& projMat = cameraTransform.projectionMat;
        meshTransform->worldMat  = meshTransform->worldMat * viewMat * projMat;
        meshTransform.ConvertToBuffer();
        meshTransform.SetForRootParameter(commandList, 0);

        // ============================= Materialのセット ============================= //
        auto& material = renderer->GetMaterialBuff();
        material.ConvertToBuffer();
        material.SetForRootParameter(commandList, 1);

        // ============================= 描画 ============================= //
        commandList->DrawIndexedInstanced(UINT(mesh.GetIndexSize()), 1, 0, 0, 0);
    }

    // 描画後クリア
    renderers.clear();
}

bool SkyboxRender::ShouldSkipRender() const {
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (!rendererByBlendMode_[i].empty()) {
            return false;
        }
    }
    return true;
}

void SkyboxRender::CreatePSO() {

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("Skybox_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (psoByBlendMode_[i]) {
                continue;
            }
            psoByBlendMode_[i] = shaderManager->GetPipelineStateObj("Skybox_" + blendModeStr[i]);
        }
        return;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("Skybox.VS");
    shaderManager->LoadShader("Skybox.PS", shaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = "Skybox.VS";
    texShaderInfo.psKey = "Skybox.PS";

    ///=================================================
    /// Depth
    ///=================================================
    texShaderInfo.CustomDepthStencilDesc().DepthEnable    = true;
    texShaderInfo.CustomDepthStencilDesc().DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 書き込まない z = -1だから
    texShaderInfo.CustomDepthStencilDesc().DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[3]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    texShaderInfo.pushBackRootParameter(rootParameter[0]);
    // Material ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[1].Descriptor.ShaderRegister = 0;
    texShaderInfo.pushBackRootParameter(rootParameter[1]);

    // Texture ... 2
    // DescriptorTable を使う
    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = texShaderInfo.pushBackRootParameter(rootParameter[2]);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister     = 0;
    descriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    texShaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);
#pragma endregion

    ///=================================================
    /// Sampler
    D3D12_STATIC_SAMPLER_DESC staticSampler = {};
    staticSampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    staticSampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.MinLOD           = 0;
    staticSampler.MaxLOD           = D3D12_FLOAT32_MAX;
    staticSampler.ShaderRegister   = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    texShaderInfo.pushBackSamplerDesc(staticSampler);
    /// Sampler
    ///=================================================

#pragma region "InputElement"

    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

#pragma endregion

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (psoByBlendMode_[i]) {
            continue;
        }
        BlendMode blend          = static_cast<BlendMode>(i);
        texShaderInfo.blendMode_ = blend;
        psoByBlendMode_[i]       = shaderManager->CreatePso("Skybox_" + blendModeStr[i], texShaderInfo, dxDevice->device_);
    }
}
