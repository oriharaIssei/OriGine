#include "SkinningMeshRenderSystem.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"

// ECS
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/material/light/LightManager.h"
#include "component/material/Material.h"
#include "component/renderer/MeshRenderer.h"
#include "component/renderer/SkyBoxRenderer.h"

using namespace OriGine;

SkinningMeshRenderSystem::SkinningMeshRenderSystem() : BaseRenderSystem() {}
SkinningMeshRenderSystem::~SkinningMeshRenderSystem() {}

void SkinningMeshRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();
}

void SkinningMeshRenderSystem::DispatchRenderer(Entity* _entity) {
    auto* skinningAnimationComponents = GetComponents<SkinningAnimationComponent>(_entity);
    if (skinningAnimationComponents == nullptr) {
        return;
    }
    auto* entityTransform = GetComponent<Transform>(_entity);

    for (auto& skinningAnimation : *skinningAnimationComponents) {
        ModelMeshRenderer* renderer = GetComponent<ModelMeshRenderer>(_entity, skinningAnimation.GetBindModeMeshRendererIndex());
        if (renderer == nullptr) {
            continue;
        }
        if (!renderer->IsRender()) {
            continue;
        }

        RenderingData data{&skinningAnimation, renderer, entityTransform};
        int32_t blendIndex = static_cast<int32_t>(renderer->GetCurrentBlend());
        activeRenderersByBlendMode_[blendIndex].push_back(data);
    }
}

bool SkinningMeshRenderSystem::ShouldSkipRender() const {
    for (const auto& renderers : activeRenderersByBlendMode_) {
        if (!renderers.empty()) {
            return false;
        }
    }
    return true;
}

void SkinningMeshRenderSystem::RenderingBy(BlendMode _blendMode, bool /*_isCulling*/) {
    int32_t blendIndex = static_cast<int32_t>(_blendMode);
    auto& renderers    = activeRenderersByBlendMode_[blendIndex];
    if (renderers.empty()) {
        return;
    }
    auto& commandList = dxCommand_->GetCommandList();
    // PSOセット
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());
    // RootSignatureセット
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());

    StartRender();

    for (auto& data : renderers) {
        RenderModelMesh(data._entityTransform, commandList, data._skinningAnimationComponent, data._renderer);
    }
}

void SkinningMeshRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void SkinningMeshRenderSystem::CreatePSO() {

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("TextureMesh_" + kBlendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (psoByBlendMode_[i]) {
                continue;
            }
            psoByBlendMode_[i] = shaderManager->GetPipelineStateObj("TextureMesh_" + kBlendModeStr[i]);
        }

        //! TODO : 自動化
        transformBufferIndex_          = 0;
        cameraBufferIndex_             = 1;
        materialBufferIndex_           = 2;
        directionalLightBufferIndex_   = 3;
        pointLightBufferIndex_         = 4;
        spotLightBufferIndex_          = 5;
        lightCountBufferIndex_         = 6;
        textureBufferIndex_            = 7;
        environmentTextureBufferIndex_ = 8;

        return;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("Object3dTexture.VS");
    shaderManager->LoadShader("Object3dTexture.PS", kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = "Object3dTexture.VS";
    texShaderInfo.psKey = "Object3dTexture.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[9]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    transformBufferIndex_                      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[0]);
    // CameraTransform ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    cameraBufferIndex_                         = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[1]);
    // Material ... 2
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    materialBufferIndex_                       = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1; // t1 register for DirectionalLight StructuredBuffer
    directionalLightBufferIndex_               = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[4].Descriptor.ShaderRegister = 3; // t3 register for PointLight StructuredBuffer
    pointLightBufferIndex_                     = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[4]);

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[5].Descriptor.ShaderRegister = 4; // t4 register for SpotLight StructuredBuffer
    spotLightBufferIndex_                      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[5]);

    // lightCounts ... 6
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    lightCountBufferIndex_                     = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[6]);

    // Texture ... 7
    // DescriptorTable を使う
    rootParameter[7].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureBufferIndex_               = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[7]);
    // 環境テクスチャ ... 8
    // DescriptorTable を使う
    rootParameter[8].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    environmentTextureBufferIndex_    = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[8]);

    D3D12_DESCRIPTOR_RANGE textureRange[1] = {};
    textureRange[0].BaseShaderRegister     = 0;
    textureRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    textureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    textureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE environmentTextureRange[1] = {};
    environmentTextureRange[0].BaseShaderRegister     = 1;
    environmentTextureRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    environmentTextureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    environmentTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE directionalLightRange[1]            = {};
    directionalLightRange[0].BaseShaderRegister                = 2;
    directionalLightRange[0].NumDescriptors                    = 1;
    directionalLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    directionalLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE pointLightRange[1]            = {};
    pointLightRange[0].BaseShaderRegister                = 3;
    pointLightRange[0].NumDescriptors                    = 1;
    pointLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pointLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE spotLightRange[1]            = {};
    spotLightRange[0].BaseShaderRegister                = 4;
    spotLightRange[0].NumDescriptors                    = 1;
    spotLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spotLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    texShaderInfo.SetDescriptorRange2Parameter(textureRange, 1, textureBufferIndex_);
    texShaderInfo.SetDescriptorRange2Parameter(environmentTextureRange, 1, environmentTextureBufferIndex_);

    texShaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, directionalLightBufferIndex_);
    texShaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, pointLightBufferIndex_);
    texShaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, spotLightBufferIndex_);
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

    inputElementDesc.SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        if (psoByBlendMode_[i]) {
            continue;
        }
        texShaderInfo.blendMode_ = blend;
        psoByBlendMode_[i]       = shaderManager->CreatePso("TextureMesh_" + kBlendModeStr[i], texShaderInfo, dxDevice->device_);
    }
}

void SkinningMeshRenderSystem::LightUpdate() {
    auto* directionalLight = GetComponentArray<DirectionalLight>();
    auto* pointLight       = GetComponentArray<PointLight>();
    auto* spotLight        = GetComponentArray<SpotLight>();

    auto* lightManager = LightManager::GetInstance();

    lightManager->ClearLights();

    for (auto& lightVec : directionalLight->GetAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive_) {
                lightManager->PushDirectionalLight(light);
            }
        }
    }
    for (auto& lightVec : pointLight->GetAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive_) {
                lightManager->PushPointLight(light);
            }
        }
    }
    for (auto& lightVec : spotLight->GetAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive_) {
                lightManager->PushSpotLight(light);
            }
        }
    }

    LightManager::GetInstance()->Update();
}

void SkinningMeshRenderSystem::StartRender() {

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    CameraManager::GetInstance()->SetBufferForRootParameter(commandList, cameraBufferIndex_);

    LightUpdate();
    LightManager::GetInstance()->SetForRootParameter(
        commandList, lightCountBufferIndex_, directionalLightBufferIndex_, pointLightBufferIndex_, spotLightBufferIndex_);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    /// 環境テクスチャ
    Entity* skyboxEntity = GetUniqueEntity("Skybox");
    if (!skyboxEntity) {
        return;
    }
    SkyboxRenderer* skybox = GetComponent<SkyboxRenderer>(skyboxEntity);
    commandList->SetGraphicsRootDescriptorTable(
        environmentTextureBufferIndex_,
        TextureManager::GetDescriptorGpuHandle(skybox->GetTextureIndex()));
}

void SkinningMeshRenderSystem::RenderModelMesh(
    Transform* _entityTransform,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
    SkinningAnimationComponent* _skinningAnimationComponent,
    ModelMeshRenderer* _renderer) {

    if (_skinningAnimationComponent->GetSkinnedVertexBuffers().empty()) {
        return;
    }

    uint32_t index = 0;

    auto& meshGroup = _renderer->GetMeshGroup();
    for (auto& mesh : *meshGroup) {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = TextureManager::GetDescriptorGpuHandle(_renderer->GetTextureNumber(index));
        IConstantBuffer<Transform>& meshTransform = _renderer->GetTransformBuff(index);
        auto& materialBuff                        = _renderer->GetMaterialBuff(index);
        Material* material                        = nullptr;
        int32_t materialIndex                     = _renderer->GetMaterialIndex(index);

        // ============================= Viewのセット ============================= //
        _commandList->IASetVertexBuffers(0, 1, &_skinningAnimationComponent->GetSkinnedVertexBuffer(index).vbView);
        _commandList->IASetIndexBuffer(&mesh.GetIBView());

        // ============================= Transformのセット ============================= //
        if (meshTransform->parent == nullptr) {
            meshTransform->parent = _entityTransform;
        }

        meshTransform->UpdateMatrix();
        meshTransform.ConvertToBuffer();
        meshTransform.SetForRootParameter(_commandList, transformBufferIndex_);

        // ============================= Materialのセット ============================= //

        if (materialIndex >= 0) {
            material = GetComponent<Material>(_renderer->GetHostEntity(), static_cast<uint32_t>(materialIndex));
            if (material) {
                material->UpdateUvMatrix();
                materialBuff.ConvertToBuffer(*material);

                if (material->hasCustomTexture()) {
                    textureHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
                }
            } else {
                materialBuff.ConvertToBuffer(Material());
            }
        }
        materialBuff.SetForRootParameter(_commandList, materialBufferIndex_);

        // ============================= テクスチャの設定 ============================= //
        _commandList->SetGraphicsRootDescriptorTable(
            textureBufferIndex_,
            textureHandle);

        // ============================= 描画 ============================= //
        _commandList->DrawIndexedInstanced(UINT(mesh.GetIndexSize()), 1, 0, 0, 0);

        ++index;
    }
}
