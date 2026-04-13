#include "TexturedMeshRenderSystem.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"
// asset
#include "asset/TextureAsset.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "asset/AssetSystem.h"

/// ECS
// entity
#include "entity/Entity.h"
// component
#include "component/material/light/LightManager.h"
#include "component/renderer/ModelMeshRenderer.h"
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
static const std::string kVSName = "Object3dTextureColor.VS";
static const std::string kPSName = "Object3dTextureColorWithRaytracing.PS";
}

TexturedMeshRenderSystem::TexturedMeshRenderSystem() : BaseRenderSystem() {}
TexturedMeshRenderSystem::~TexturedMeshRenderSystem() {};

void TexturedMeshRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();
    instancedMeshManager_.Initialize();
}

void OriGine::TexturedMeshRenderSystem::Update() {
    // レンダリング対象が無ければスキップ
    if (entities_.empty()) {
        return;
    }

    // 死んだエンティティのインスタンスをクリーンアップ
    for (auto& entityID : entities_) {
        Entity* entity = GetEntity(entityID);
        if (!entityID.IsValid() || !entity || !entity->IsAlive()) {
            // ModelMeshRenderer のインスタンスハンドルを解放
            auto& modelRenderers = GetComponents<ModelMeshRenderer>(entityID);
            for (auto& renderer : modelRenderers) {
                if (renderer.GetInstanceHandle().IsValid()) {
                    instancedMeshManager_.RemoveInstance(renderer.GetInstanceHandle());
                    renderer.SetInstanceHandle(InstanceHandle{});
                }
            }

            // Primitive のインスタンスハンドルを解放
            auto cleanupPrimitive = [this](auto& renderers) {
                for (auto& renderer : renderers) {
                    if (renderer.GetInstanceHandle().IsValid()) {
                        instancedMeshManager_.RemoveInstance(renderer.GetInstanceHandle());
                        renderer.SetInstanceHandle(InstanceHandle{});
                    }
                }
            };
            cleanupPrimitive(GetComponents<PlaneRenderer>(entityID));
            cleanupPrimitive(GetComponents<BoxRenderer>(entityID));
            cleanupPrimitive(GetComponents<RingRenderer>(entityID));
            cleanupPrimitive(GetComponents<SphereRenderer>(entityID));
            cleanupPrimitive(GetComponents<CylinderRenderer>(entityID));
        }
    }

    // 有効でないエンティティを削除
    EraseDeadEntity();

    // カウントプレパス: 同一モデルのエンティティ数を集計
    modelInstanceCounts_.clear();
    for (auto& entityID : entities_) {
        auto& modelMeshRenderers = GetComponents<ModelMeshRenderer>(entityID);
        for (auto& renderer : modelMeshRenderers) {
            ModelMeshData* modelData = renderer.GetModelData();
            if (modelData && !renderer.IsForceNonInstanced()) {
                modelInstanceCounts_[modelData]++;
            }
        }
    }

    // レンダラー登録
    for (auto& entityID : entities_) {
        DispatchRenderer(entityID);
    }

    // レンダリングスキップ判定
    if (ShouldSkipRender()) {
        return;
    }

    // RaytracingSceneの更新
    ///! TODO: シーン側で更新するべき
    // だけど、メッシュのTransformをここで更新しているので、暫定対応としておく
    GetScene()->UpdateRaytracingScene();
    // インスタンシング描画のバッファ更新
    instancedMeshManager_.UpdateBuffers(GetScene());

    // レンダリング実行
    Rendering();
}

void TexturedMeshRenderSystem::DispatchRenderer(EntityHandle _entity) {
    auto entityTransform = GetComponent<Transform>(_entity);

    if (entityTransform) {
        entityTransform->UpdateMatrix();
    }

    //==============================
    // ModelMeshRenderer の振り分け
    //==============================
    auto& modelMeshRenderers = GetComponents<ModelMeshRenderer>(_entity);
    if (!modelMeshRenderers.empty()) {

        for (auto& renderer : modelMeshRenderers) {
            if (renderer.GetMeshGroup()->empty() || !renderer.IsRender()) {
                continue;
            }

            ModelMeshData* modelData = renderer.GetModelData();
            auto countItr = modelData ? modelInstanceCounts_.find(modelData) : modelInstanceCounts_.end();
            bool shouldInstance = modelData
                && !renderer.IsForceNonInstanced()
                && countItr != modelInstanceCounts_.end()
                && countItr->second >= kAutoInstanceThreshold;

            if (shouldInstance) {
                // インスタンシング描画パス
                if (!renderer.GetInstanceHandle().IsValid()) {
                    renderer.SetInstanceHandle(
                        instancedMeshManager_.AddInstance(modelData, _entity));
                }

                // Material / Texture / BlendMode / Culling 同期
                InstanceEntry& entry = instancedMeshManager_.GetInstance(renderer.GetInstanceHandle());
                entry.blendMode  = renderer.GetCurrentBlend();
                entry.isCulling  = renderer.IsCulling();
                uint32_t submeshCount = static_cast<uint32_t>(renderer.GetMeshGroup()->size());
                for (uint32_t i = 0; i < submeshCount; ++i) {
                    ComponentHandle matHandle = renderer.GetMaterialHandle(i);
                    Material* material = GetComponent<Material>(matHandle);
                    if (material && i < entry.materials.size()) {
                        material->UpdateUvMatrix();
                        entry.materials[i] = *material;

                        // customTexture の同期
                        if (i < entry.customTextureHandles.size()) {
                            if (material->hasCustomTexture()) {
                                entry.customTextureHandles[i] = material->GetCustomTexture()->srv_.GetGpuHandle();
                            } else {
                                entry.customTextureHandles[i] = D3D12_GPU_DESCRIPTOR_HANDLE{0};
                            }
                        }
                    }
                    if (i < entry.textureIndices.size()) {
                        entry.textureIndices[i] = renderer.GetTextureIndex(i);
                    }
                }
            } else {
                // 閾値未満に戻った場合：インスタンスを解除
                if (renderer.GetInstanceHandle().IsValid()) {
                    instancedMeshManager_.RemoveInstance(renderer.GetInstanceHandle());
                    renderer.SetInstanceHandle(InstanceHandle{});
                }

                // 個別描画パス
                for (int32_t i = 0; i < static_cast<int32_t>(renderer.GetMeshGroup()->size()); ++i) {
                    auto& transform = renderer.GetTransformBuff(i);
                    if (transform->parent == nullptr) {
                        transform->parent = entityTransform;
                    }
                    transform->UpdateMatrix();
                    transform.ConvertToBuffer();
                }

                BlendMode blendMode = renderer.GetCurrentBlend();
                int32_t blendIndex  = static_cast<int32_t>(blendMode);
                int32_t isCulling   = renderer.IsCulling() ? 1 : 0;
                activeModelMeshRenderer_[isCulling][blendIndex].push_back(&renderer);
            }
        }
    }

    //==============================
    // Primitive の振り分け
    //==============================
    auto dispatchPrimitive = [this, _entity, entityTransform](auto& renderers) {
        for (auto& renderer : renderers) {
            if (!renderer.IsRender()) {
                continue;
            }

            // インスタンシング描画判定
            if (renderer.IsInstancing()) {
                if (!renderer.GetInstanceHandle().IsValid()) {
                    // テンプレートメッシュを取得し、インスタンスを追加
                    auto templateMesh = instancedMeshManager_.GetOrCreatePrimitiveTemplate(
                        typeid(renderer).name(),
                        [&renderer](TextureColorMesh* _mesh) {
                            renderer.CreateMesh(_mesh);
                        });
                    renderer.SetInstanceHandle(
                        instancedMeshManager_.AddInstance(templateMesh, _entity));
                }

                // Material / Texture / LocalTransform / BlendMode / Culling 同期
                InstanceEntry& entry = instancedMeshManager_.GetInstance(renderer.GetInstanceHandle());
                entry.blendMode  = renderer.GetCurrentBlend();
                entry.isCulling  = renderer.IsCulling();
                if (!entry.materials.empty()) {
                    Material* material = GetComponent<Material>(_entity, renderer.GetMaterialIndex());
                    if (material) {
                        material->UpdateUvMatrix();
                        entry.materials[0] = *material;

                        // customTexture の同期
                        if (!entry.customTextureHandles.empty()) {
                            if (material->hasCustomTexture()) {
                                entry.customTextureHandles[0] = material->GetCustomTexture()->srv_.GetGpuHandle();
                            } else {
                                entry.customTextureHandles[0] = D3D12_GPU_DESCRIPTOR_HANDLE{0};
                            }
                        }
                    }
                }
                if (!entry.textureIndices.empty()) {
                    entry.textureIndices[0] = renderer.GetTextureIndex();
                }
                // ローカル Transform 同期（レンダラーの transformBuff_ を使用）
                if (!entry.localTransforms.empty()) {
                    entry.localTransforms[0] = renderer.GetTransformBuff().openData_;
                }
                continue;
            }

            // 個別描画パス
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

            // lighting有効なマテリアルのみ、RaytracingSceneへ登録する
            Material* meshMaterial = GetComponent<Material>(_entity, renderer.GetMaterialIndex());
            if (!meshMaterial || !meshMaterial->enableLighting_) {
                continue;
            }
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

    bool hasNonInstanced = !activeModelMeshRenderers.empty() || !activePrimitiveMeshRenderers.empty();
    bool hasInstanced    = instancedMeshManager_.HasInstancesFor(_blendMode, _isCulling);

    if (!hasNonInstanced && !hasInstanced) {
        return;
    }

    auto& commandList = dxCommand_->GetCommandList();
    currentCulling_   = _isCulling;
    currentBlendMode_ = _blendMode;

    // 非インスタンス描画
    if (hasNonInstanced) {
        StartRender();

        // model
        for (auto& renderer : activeModelMeshRenderers) {
            RenderModelMesh(commandList, renderer);
        }
        activeModelMeshRenderers.clear();

        // primitive
        for (auto& renderer : activePrimitiveMeshRenderers) {
            RenderPrimitiveMesh(commandList, renderer);
        }
        activePrimitiveMeshRenderers.clear();
    }

    // インスタンシング描画（該当する BlendMode / Culling のみ）
    if (hasInstanced) {
        // インスタンシング用 PSO に切り替え
        commandList->SetGraphicsRootSignature(instancedPsoByBlendMode_[cullingIndex][blendIndex]->rootSignature.Get());
        commandList->SetPipelineState(instancedPsoByBlendMode_[cullingIndex][blendIndex]->pipelineState.Get());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
        commandList->SetDescriptorHeaps(1, ppHeaps);

        // Camera
        CameraManager* cameraManager = CameraManager::GetInstance();
        cameraManager->SetBufferForRootParameter(GetScene(), commandList, cameraBufferIndex_);

        // Light
        LightManager::GetInstance()->SetForRootParameter(
            commandList, lightCountBufferIndex_, directionalLightBufferIndex_, pointLightBufferIndex_, spotLightBufferIndex_);

        // RaytracingScene
        if (!GetScene()->GetRaytracingScene()->IsEmpty()) {
            commandList->SetGraphicsRootShaderResourceView(raytracingSceneBufferIndex_, GetScene()->GetRaytracingSceneRef()->GetTlasResource()->GetGPUVirtualAddress());
        }

        // 環境テクスチャ
        EntityHandle skyboxEntity = GetUniqueEntity("Skybox");
        if (skyboxEntity.IsValid()) {
            SkyboxRenderer* skybox = GetComponent<SkyboxRenderer>(skyboxEntity);
            commandList->SetGraphicsRootDescriptorTable(
                environmentTextureBufferIndex_,
                AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(skybox->GetTextureIndex()).srv.GetGpuHandle());
        }

        // インスタンシング描画
        instancedMeshManager_.Render(
            commandList,
            instancedTransformBufferIndex_,
            instancedMaterialBufferIndex_,
            textureBufferIndex_,
            instancedOffsetBufferIndex_,
            _blendMode,
            _isCulling);
    }
}

bool TexturedMeshRenderSystem::ShouldSkipRender() const {
    // インスタンシング描画がある場合はスキップしない
    if (!instancedMeshManager_.IsEmpty()) {
        return false;
    }
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
    for (auto& activeRendererByCulling : activeModelMeshRenderer_) {
        for (auto& activeRenderers : activeRendererByCulling) {
            activeRenderers.clear();
        }
    }
    for (auto& activeRendererByCulling : activePrimitiveMeshRenderer_) {
        for (auto& activeRenderers : activeRendererByCulling) {
            activeRenderers.clear();
        }
    }

    instancedMeshManager_.Finalize();
    dxCommand_->Finalize();
}

void TexturedMeshRenderSystem::CreatePSO() {
    const std::string kPsoKey        = "TextureMeshWithRaytracing_";
    const std::string kCullingPsoKey = "CullingTextureMeshWithRaytracing_";

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
        raytracingSceneBufferIndex_    = 9;

        // すべて登録されていれば return
        if (isAllRegistered) {
            return;
        }
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader(kVSName);
    shaderManager->LoadShader(kPSName, kShaderDirectory, L"ps_6_5");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = kVSName;
    texShaderInfo.psKey = kPSName;

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[10]{};
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

    rootParameter[3].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    directionalLightBufferIndex_      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    pointLightBufferIndex_            = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[4]));

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    spotLightBufferIndex_             = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[5]));

    // lightCounts ... 6
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    lightCountBufferIndex_                     = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[6]));

    // Texture ... 7
    // DescriptorTable を使う
    rootParameter[7].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureBufferIndex_               = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[7]));
    // 環境テクスチャ ... 8
    // DescriptorTable を使う
    rootParameter[8].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    environmentTextureBufferIndex_    = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[8]));

    // raytracingScene ... 9
    rootParameter[9].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[9].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[9].Descriptor.ShaderRegister = 5; // t5
    rootParameter[9].Descriptor.RegisterSpace  = 0;
    raytracingSceneBufferIndex_                = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[9]));

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

    // インスタンシング用 PSO の作成
    CreateInstancedPSO();
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
                if (light.isActive) {
                    lightManager->PushDirectionalLight(light);
                }
            }
        }
    }

    if (pointLight) {
        for (auto& lightVec : pointLight->GetSlots()) {
            for (auto& light : lightVec.components) {
                if (light.isActive) {
                    lightManager->PushPointLight(light);
                }
            }
        }
    }

    if (spotLight) {
        for (auto& lightVec : spotLight->GetSlots()) {
            for (auto& light : lightVec.components) {
                if (light.isActive) {
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

    // RaytracingSceneのセット
    if (!GetScene()->GetRaytracingScene()->IsEmpty()) {
        commandList->SetGraphicsRootShaderResourceView(raytracingSceneBufferIndex_, GetScene()->GetRaytracingSceneRef()->GetTlasResource()->GetGPUVirtualAddress());
    }

    /// 環境テクスチャ
    EntityHandle skyboxEntity = GetUniqueEntity("Skybox");
    if (!skyboxEntity.IsValid()) {
        return;
    }
    SkyboxRenderer* skybox = GetComponent<SkyboxRenderer>(skyboxEntity);
    commandList->SetGraphicsRootDescriptorTable(
        environmentTextureBufferIndex_,
        AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(skybox->GetTextureIndex()).srv.GetGpuHandle());
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
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle       = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(_renderer->GetTextureIndex(index)).srv.GetGpuHandle();
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
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(_renderer->GetTextureIndex()).srv.GetGpuHandle();

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

//==============================================================================
// TexturedMeshRenderSystem - Instanced PSO
//==============================================================================

void TexturedMeshRenderSystem::CreateInstancedPSO() {
    const std::string kInstancedVSName = "Object3dTextureColorInstanced.VS";
    const std::string kInstancedPSName = "Object3dTextureColorInstancedWithRaytracing.PS";
    const std::string kInstancedPsoKey         = "InstancedTextureMeshWithRaytracing_";
    const std::string kInstancedCullingPsoKey   = "CullingInstancedTextureMeshWithRaytracing_";

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    // 登録済みチェック
    if (shaderManager->IsRegisteredPipelineStateObj(kInstancedPsoKey + kBlendModeStr[0])) {
        bool isAllRegistered = true;
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (!instancedPsoByBlendMode_[0][i] || !instancedPsoByBlendMode_[1][i]) {
                isAllRegistered = false;
                continue;
            }
            instancedPsoByBlendMode_[0][i] = shaderManager->GetPipelineStateObj(kInstancedPsoKey + kBlendModeStr[i]);
            instancedPsoByBlendMode_[1][i] = shaderManager->GetPipelineStateObj(kInstancedCullingPsoKey + kBlendModeStr[i]);
        }

        // インスタンシング用ルートパラメータインデックスを設定
        instancedTransformBufferIndex_ = 0;
        instancedMaterialBufferIndex_  = 2;
        instancedOffsetBufferIndex_    = 10;

        if (isAllRegistered) {
            return;
        }
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader(kInstancedVSName);
    shaderManager->LoadShader(kInstancedPSName, kShaderDirectory, L"ps_6_5");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo shaderInfo{};
    shaderInfo.vsKey = kInstancedVSName;
    shaderInfo.psKey = kInstancedPSName;

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[11]{};

    // [0] Transform StructuredBuffer (DescriptorTable, SRV t6)
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    instancedTransformBufferIndex_    = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[0]));

    // [1] Camera/ViewProjection (CBV b2)
    // NOTE: 共有パラメータのインデックスは非インスタンスPSOと同じ順序で配置されている.
    //       共有メンバ変数は上書きせず、ローカル変数で受ける.
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    [[maybe_unused]] int32_t instCameraIdx     = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[1]));

    // [2] Material StructuredBuffer (DescriptorTable, SRV t7)
    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    instancedMaterialBufferIndex_     = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[2]));

    // [3] DirectionalLight (DescriptorTable)
    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    [[maybe_unused]] int32_t instDirLightIdx   = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[3]));

    // [4] PointLight (DescriptorTable)
    rootParameter[4].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;
    [[maybe_unused]] int32_t instPointLightIdx  = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[4]));

    // [5] SpotLight (DescriptorTable)
    rootParameter[5].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;
    [[maybe_unused]] int32_t instSpotLightIdx   = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[5]));

    // [6] LightCounts (CBV b5)
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    [[maybe_unused]] int32_t instLightCountIdx = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[6]));

    // [7] Texture (DescriptorTable, SRV t0)
    rootParameter[7].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[7].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    [[maybe_unused]] int32_t instTextureIdx    = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[7]));

    // [8] Environment Texture (DescriptorTable, SRV t1)
    rootParameter[8].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[8].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    [[maybe_unused]] int32_t instEnvTexIdx     = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[8]));

    // [9] RaytracingScene (SRV t5)
    rootParameter[9].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[9].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[9].Descriptor.ShaderRegister = 5; // t5
    rootParameter[9].Descriptor.RegisterSpace  = 0;
    [[maybe_unused]] int32_t instRaytracingIdx = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[9]));

    // [10] InstanceOffset (Root Constants, b6) - テクスチャバッチ描画時のオフセット
    rootParameter[10].ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameter[10].ShaderVisibility         = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[10].Constants.ShaderRegister = 6; // b6
    rootParameter[10].Constants.RegisterSpace  = 0;
    rootParameter[10].Constants.Num32BitValues = 1;
    instancedOffsetBufferIndex_                = static_cast<int32_t>(shaderInfo.pushBackRootParameter(rootParameter[10]));

    // DescriptorRange 定義
    // Transform StructuredBuffer (t6)
    D3D12_DESCRIPTOR_RANGE instanceTransformRange[1]            = {};
    instanceTransformRange[0].BaseShaderRegister                = 6;
    instanceTransformRange[0].NumDescriptors                    = 1;
    instanceTransformRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceTransformRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Material StructuredBuffer (t7)
    D3D12_DESCRIPTOR_RANGE instanceMaterialRange[1]            = {};
    instanceMaterialRange[0].BaseShaderRegister                = 7;
    instanceMaterialRange[0].NumDescriptors                    = 1;
    instanceMaterialRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceMaterialRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Texture (t0)
    D3D12_DESCRIPTOR_RANGE textureRange[1]            = {};
    textureRange[0].BaseShaderRegister                = 0;
    textureRange[0].NumDescriptors                    = 1;
    textureRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Environment Texture (t1)
    D3D12_DESCRIPTOR_RANGE environmentTextureRange[1]            = {};
    environmentTextureRange[0].BaseShaderRegister                = 1;
    environmentTextureRange[0].NumDescriptors                    = 1;
    environmentTextureRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    environmentTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Light SRVs
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

    // DescriptorRange をルートパラメータに紐付け
    shaderInfo.SetDescriptorRange2Parameter(instanceTransformRange, 1, instancedTransformBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(instanceMaterialRange, 1, instancedMaterialBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(textureRange, 1, textureBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(environmentTextureRange, 1, environmentTextureBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, directionalLightBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, pointLightBufferIndex_);
    shaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, spotLightBufferIndex_);
#pragma endregion

    ///=================================================
    /// Sampler
    D3D12_STATIC_SAMPLER_DESC staticSampler = {};
    staticSampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.MinLOD           = 0;
    staticSampler.MaxLOD           = D3D12_FLOAT32_MAX;
    staticSampler.ShaderRegister   = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(staticSampler);

#pragma region "InputElement"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION";
    inputElementDesc.SemanticIndex            = 0;
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "TEXCOORD";
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL";
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "COLOR";
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// BlendMode ごとの PSO を作成
    ///=================================================

    // カリングなし
    shaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (instancedPsoByBlendMode_[0][i] != nullptr) {
            continue;
        }
        shaderInfo.blendMode_          = static_cast<BlendMode>(i);
        instancedPsoByBlendMode_[0][i] = shaderManager->CreatePso(kInstancedPsoKey + kBlendModeStr[i], shaderInfo, dxDevice->device_);
    }

    // カリングあり
    shaderInfo.changeCullMode(D3D12_CULL_MODE_BACK);
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (instancedPsoByBlendMode_[1][i] != nullptr) {
            continue;
        }
        shaderInfo.blendMode_          = static_cast<BlendMode>(i);
        instancedPsoByBlendMode_[1][i] = shaderManager->CreatePso(kInstancedCullingPsoKey + kBlendModeStr[i], shaderInfo, dxDevice->device_);
    }
}
