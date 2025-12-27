#include "TexturedMeshRenderSystem.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"

/// ECS
// component
#include "component/material/light/LightManager.h"
#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/SkyboxRenderer.h"
// primitives
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

using namespace OriGine;

namespace {
static const std::string kShaderName = "Object3dTextureColor";
}

TexturedMeshRenderSystem::TexturedMeshRenderSystem() : BaseRenderSystem() {}
TexturedMeshRenderSystem::~TexturedMeshRenderSystem() {};

void TexturedMeshRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();
}

void TexturedMeshRenderSystem::DispatchRenderer(EntityHandle _entity) {
    auto entityTransform = GetComponent<Transform>(_entity);

    if (entityTransform) {
        entityTransform->UpdateMatrix();
    }

    auto& modelMeshRenderers = GetComponents<ModelMeshRenderer>(_entity);
    if (!modelMeshRenderers.empty()) {

        for (auto& renderer : modelMeshRenderers) {
            if (renderer.GetMeshGroup()->empty() || !renderer.IsRender()) {
                continue;
            }

            for (int32_t i = 0; i < static_cast<int32_t>(renderer.GetMeshGroup()->size()); ++i) {
                ///==============================
                /// Transformの更新 (meshごと)
                ///==============================
                auto& transform = renderer.GetTransformBuff(i);

                if (transform->parent == nullptr) {
                    transform->parent = entityTransform;
                }

                transform->UpdateMatrix();
                transform.ConvertToBuffer();
            }

            ///==============================
            /// push_back
            ///==============================
            BlendMode blendMode = renderer.GetCurrentBlend();
            int32_t blendIndex  = static_cast<int32_t>(blendMode);
            int32_t isCulling   = renderer.IsCulling() ? 1 : 0;
            activeModelMeshRenderer_[isCulling][blendIndex].push_back(&renderer);
        }
    }

    auto dispatchPrimitive = [this, _entity, entityTransform](auto& renderers) {
        for (auto& renderer : renderers) {
            if (!renderer.IsRender()) {
                continue;
            }

            ///==============================
            /// Transformの更新
            ///==============================
            auto& transform = renderer.GetTransformBuff();

            if (transform->parent == nullptr) {
                transform->parent = entityTransform;
            }

            transform->UpdateMatrix();
            transform.ConvertToBuffer();

            BlendMode blendMode = renderer.GetCurrentBlend();
            int32_t blendIndex  = static_cast<int32_t>(blendMode);
            int32_t isCulling   = renderer.IsCulling() ? 1 : 0;
            activePrimitiveMeshRenderer_[isCulling][blendIndex].push_back(&renderer);
        }
    };

    dispatchPrimitive(GetComponents<PlaneRenderer>(_entity));
    dispatchPrimitive(GetComponents<RingRenderer>(_entity));
    dispatchPrimitive(GetComponents<BoxRenderer>(_entity));
    dispatchPrimitive(GetComponents<SphereRenderer>(_entity));
    dispatchPrimitive(GetComponents<CylinderRenderer>(_entity));
}

void TexturedMeshRenderSystem::RenderingBy(BlendMode _blendMode, bool _isCulling) {
    int32_t cullingIndex = _isCulling ? 1 : 0;
    int32_t blendIndex   = static_cast<int32_t>(_blendMode);

    auto& activeModelMeshRenderers     = activeModelMeshRenderer_[cullingIndex][blendIndex];
    auto& activePrimitiveMeshRenderers = activePrimitiveMeshRenderer_[cullingIndex][blendIndex];

    bool isSkip = activeModelMeshRenderers.empty() && activePrimitiveMeshRenderers.empty();
    if (isSkip) {
        return;
    }

    auto& commandList = dxCommand_->GetCommandList();
    currentCulling_   = _isCulling;
    currentBlendMode_ = _blendMode;

    StartRender();

    // model
    if (!activeModelMeshRenderers.empty()) {
        for (auto& renderer : activeModelMeshRenderers) {
            RenderModelMesh(commandList, renderer);
        }
        activeModelMeshRenderers.clear();
    }
    // primitive
    if (!activePrimitiveMeshRenderers.empty()) {
        for (auto& renderer : activePrimitiveMeshRenderers) {
            RenderPrimitiveMesh(commandList, renderer);
        }
        activePrimitiveMeshRenderers.clear();
    }
}

bool TexturedMeshRenderSystem::ShouldSkipRender() const {
    for (size_t isCulling = 0; isCulling < 2; ++isCulling) {
        for (size_t blendIndex = 0; blendIndex < kBlendNum; ++blendIndex) {
            if (!activeModelMeshRenderer_[isCulling][blendIndex].empty() || !activePrimitiveMeshRenderer_[isCulling][blendIndex].empty()) {
                return false;
            }
        }
    }
    return true;
}

void TexturedMeshRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void TexturedMeshRenderSystem::CreatePSO() {
    const std::string kPsoKey        = "TextureMesh_";
    const std::string kCullingPsoKey = "CullingTextureMesh_";

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj(kPsoKey + kBlendModeStr[0])) {
        bool isAllRegistered = true;
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (!psoByBlendMode_[0][i] || !psoByBlendMode_[1][i]) {
                isAllRegistered = false;
                continue;
            }
            psoByBlendMode_[0][i] = shaderManager->GetPipelineStateObj(kPsoKey + kBlendModeStr[i]);
            psoByBlendMode_[1][i] = shaderManager->GetPipelineStateObj(kCullingPsoKey + kBlendModeStr[i]);
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

        // すべて登録されていれば return
        if (isAllRegistered) {
            return;
        }
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader(kShaderName + ".VS");
    shaderManager->LoadShader(kShaderName + ".PS", kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = kShaderName + ".VS";
    texShaderInfo.psKey = kShaderName + ".PS";

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

    inputElementDesc.SemanticName      = "COLOR"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

#pragma endregion

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================

    // カリングなし
    texShaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        if (psoByBlendMode_[0][i] != nullptr) {
            continue;
        }
        texShaderInfo.blendMode_ = blend;
        psoByBlendMode_[0][i]    = shaderManager->CreatePso(kPsoKey + kBlendModeStr[i], texShaderInfo, dxDevice->device_);
    }

    // カリングあり
    texShaderInfo.changeCullMode(D3D12_CULL_MODE_BACK);
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        if (psoByBlendMode_[1][i] != nullptr) {
            continue;
        }
        texShaderInfo.blendMode_ = blend;
        psoByBlendMode_[1][i]    = shaderManager->CreatePso(kCullingPsoKey + kBlendModeStr[i], texShaderInfo, dxDevice->device_);
    }
}

void TexturedMeshRenderSystem::LightUpdate() {
    auto* directionalLight = GetComponentArray<DirectionalLight>();
    auto* pointLight       = GetComponentArray<PointLight>();
    auto* spotLight        = GetComponentArray<SpotLight>();

    auto* lightManager = LightManager::GetInstance();

    lightManager->ClearLights();

    if (directionalLight) {
        for (auto& lightVec : directionalLight->GetSlots()) {
            for (auto& light : lightVec.components) {
                if (light.isActive_) {
                    lightManager->PushDirectionalLight(light);
                }
            }
        }
    }

    if (pointLight) {
        for (auto& lightVec : pointLight->GetSlots()) {
            for (auto& light : lightVec.components) {
                if (light.isActive_) {
                    lightManager->PushPointLight(light);
                }
            }
        }
    }

    if (spotLight) {
        for (auto& lightVec : spotLight->GetSlots()) {
            for (auto& light : lightVec.components) {
                if (light.isActive_) {
                    lightManager->PushSpotLight(light);
                }
            }
        }
    }

    LightManager::GetInstance()->Update();
}

void TexturedMeshRenderSystem::StartRender() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    int32_t cullingIndex = currentCulling_ ? 1 : 0;
    int32_t blendIndex   = static_cast<int32_t>(currentBlendMode_);

    // PSOとRootSignatureの設定(パラメーターを設定するため,とりあえずPSOをセット)
    commandList->SetGraphicsRootSignature(psoByBlendMode_[cullingIndex][blendIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[cullingIndex][blendIndex]->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // Cameraのセット
    CameraManager* cameraManager = CameraManager::GetInstance();
    cameraManager->DataConvertToBuffer(GetScene());
    cameraManager->SetBufferForRootParameter(GetScene(), commandList, cameraBufferIndex_);

    // Lightのセット
    LightUpdate();
    LightManager::GetInstance()->SetForRootParameter(
        commandList, lightCountBufferIndex_, directionalLightBufferIndex_, pointLightBufferIndex_, spotLightBufferIndex_);

    /// 環境テクスチャ
    EntityHandle skyboxEntity = GetUniqueEntity("Skybox");
    if (!skyboxEntity.IsValid()) {
        return;
    }
    SkyboxRenderer* skybox = GetComponent<SkyboxRenderer>(skyboxEntity);
    commandList->SetGraphicsRootDescriptorTable(
        environmentTextureBufferIndex_,
        TextureManager::GetDescriptorGpuHandle(skybox->GetTextureIndex()));
}

void TexturedMeshRenderSystem::RenderingMesh(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
    const TextureColorMesh& _mesh,
    IConstantBuffer<Transform>& _transformBuff,
    IConstantBuffer<Material>& _materialBuff,
    D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const {
    // ============================= テクスチャの設定 ============================= //

    _commandList->SetGraphicsRootDescriptorTable(
        textureBufferIndex_,
        _textureHandle);

    // ============================= Viewのセット ============================= //
    _commandList->IASetVertexBuffers(0, 1, &_mesh.GetVBView());
    _commandList->IASetIndexBuffer(&_mesh.GetIBView());

    // ============================= Transformのセット ============================= //
    _transformBuff->UpdateMatrix();
    _transformBuff.ConvertToBuffer();
    _transformBuff.SetForRootParameter(_commandList, transformBufferIndex_);

    // ============================= Materialのセット ============================= //
    _materialBuff->UpdateUvMatrix();
    _materialBuff.ConvertToBuffer();
    _materialBuff.SetForRootParameter(_commandList, materialBufferIndex_);

    // ============================= 描画 ============================= //
    _commandList->DrawIndexedInstanced(UINT(_mesh.GetIndexSize()), 1, 0, 0, 0);
}

void TexturedMeshRenderSystem::RenderingMesh(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
    const TextureColorMesh& _mesh,
    IConstantBuffer<Transform>& _transformBuff,
    SimpleConstantBuffer<Material>& _materialBuff,
    D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle) const {
    // ============================= テクスチャの設定 ============================= //

    _commandList->SetGraphicsRootDescriptorTable(
        textureBufferIndex_,
        _textureHandle);

    // ============================= Viewのセット ============================= //
    _commandList->IASetVertexBuffers(0, 1, &_mesh.GetVBView());
    _commandList->IASetIndexBuffer(&_mesh.GetIBView());

    // ============================= Transformのセット ============================= //
    _transformBuff->UpdateMatrix();
    _transformBuff.ConvertToBuffer();
    _transformBuff.SetForRootParameter(_commandList, transformBufferIndex_);

    // ============================= Materialのセット ============================= //
    _materialBuff.SetForRootParameter(_commandList, materialBufferIndex_);

    // ============================= 描画 ============================= //
    _commandList->DrawIndexedInstanced(UINT(_mesh.GetIndexSize()), 1, 0, 0, 0);
}

void TexturedMeshRenderSystem::RenderModelMesh(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ModelMeshRenderer* _renderer) {
    uint32_t index = 0;

    auto& meshGroup = _renderer->GetMeshGroup();
    for (auto& mesh : *meshGroup) {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle       = TextureManager::GetDescriptorGpuHandle(_renderer->GetTextureNumber(index));
        const IConstantBuffer<Transform>& meshTransform = _renderer->GetTransformBuff(index);
        auto& materialBuff                              = _renderer->GetMaterialBuff(index);
        Material* material                              = nullptr;
        ComponentHandle materialHandle                  = _renderer->GetMaterialHandle(index);

        // ============================= Viewのセット ============================= //
        _commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
        _commandList->IASetIndexBuffer(&mesh.GetIBView());

        // ============================= Transformのセット ============================= //
        meshTransform.ConvertToBuffer();
        meshTransform.SetForRootParameter(_commandList, transformBufferIndex_);

        // ============================= Materialのセット ============================= //
        material = GetComponent<Material>(materialHandle);
        if (material) {
            material->UpdateUvMatrix();
            materialBuff.ConvertToBuffer(*material);

            if (material->hasCustomTexture()) {
                textureHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
            }
        }

        materialBuff.SetForRootParameter(_commandList, materialBufferIndex_);

        // ============================= テクスチャの設定 ============================= //

        _commandList->SetGraphicsRootDescriptorTable(
            textureBufferIndex_, textureHandle);

        // ============================= 描画 ============================= //
        _commandList->DrawIndexedInstanced(UINT(mesh.GetIndexSize()), 1, 0, 0, 0);

        ++index;
    }
}

void TexturedMeshRenderSystem::RenderPrimitiveMesh(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList,
    PrimitiveMeshRendererBase* _renderer) {

    for (const auto& mesh : *_renderer->GetMeshGroup()) {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = TextureManager::GetDescriptorGpuHandle(_renderer->GetTextureIndex());

        auto& materialBuff    = _renderer->GetMaterialBuff();
        Material* material    = nullptr;
        int32_t materialIndex = _renderer->GetMaterialIndex();
        // ============================= Materialのセット ============================= //
        if (materialIndex >= 0) {
            material = GetComponent<Material>(_renderer->GetHostEntityHandle(), static_cast<uint32_t>(materialIndex));
            if (material) {
                material->UpdateUvMatrix();
                materialBuff.ConvertToBuffer(*material);

                if (material->hasCustomTexture()) {
                    textureHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
                }
            }
        }

        this->RenderingMesh(
            _commandList,
            mesh,
            _renderer->GetTransformBuff(),
            _renderer->GetMaterialBuff(),
            textureHandle);
    }
}
