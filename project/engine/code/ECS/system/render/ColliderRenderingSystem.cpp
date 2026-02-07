#include "ColliderRenderingSystem.h"
#include "ColliderMeshBuilder.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"

// ECS
// component
#include "component/material/light/LightManager.h"
#include "component/renderer/MeshRenderer.h"
#include "component/transform/Transform.h"

/// math
#include "math/bounds/base/IBounds.h"
#include <Matrix4x4.h>
#include <numbers>

using namespace OriGine;
using namespace OriGine::ColliderMesh;

const int32_t ColliderRenderingSystem::kDefaultMeshCount_ = 1000;

ColliderRenderingSystem::ColliderRenderingSystem() : BaseRenderSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
ColliderRenderingSystem::~ColliderRenderingSystem() {}

/// <summary>
/// 初期化処理。各形状用レンダラーの生成を行う。
/// </summary>
void ColliderRenderingSystem::Initialize() {
    BaseRenderSystem::Initialize();

    //** AABB **//
    aabbColliders_ = GetComponentArray<AABBCollider>();
    aabbRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    aabbRenderer_->Initialize(nullptr, EntityHandle());
    aabbRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    aabbRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kAabbVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kAabbIndexSize);
    aabbMeshItr_ = aabbRenderer_->GetMeshGroup()->begin();

    //** OBB **//
    obbColliders_ = GetComponentArray<OBBCollider>();
    obbRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    obbRenderer_->Initialize(nullptr, EntityHandle());
    obbRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    obbRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kObbVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kObbIndexSize);
    obbMeshItr_ = obbRenderer_->GetMeshGroup()->begin();

    //** Sphere **//
    sphereColliders_ = GetComponentArray<SphereCollider>();
    sphereRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    sphereRenderer_->Initialize(nullptr, EntityHandle());
    sphereRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    sphereRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kSphereVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kSphereIndexSize);
    sphereMeshItr_ = sphereRenderer_->GetMeshGroup()->begin();

    //** Ray **//
    rayColliders_ = GetComponentArray<RayCollider>();
    rayRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    rayRenderer_->Initialize(nullptr, EntityHandle());
    rayRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    rayRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kRayVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kRayIndexSize);
    rayMeshItr_ = rayRenderer_->GetMeshGroup()->begin();

    //** Segment **//
    segmentColliders_ = GetComponentArray<SegmentCollider>();
    segmentRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    segmentRenderer_->Initialize(nullptr, EntityHandle());
    segmentRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    segmentRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kSegmentVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kSegmentIndexSize);
    segmentMeshItr_ = segmentRenderer_->GetMeshGroup()->begin();

    //** Capsule **//
    capsuleColliders_ = GetComponentArray<CapsuleCollider>();
    capsuleRenderer_  = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
    capsuleRenderer_->Initialize(nullptr, EntityHandle());
    capsuleRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    capsuleRenderer_->GetMeshGroup()->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kCapsuleVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kCapsuleIndexSize);
    capsuleMeshItr_ = capsuleRenderer_->GetMeshGroup()->begin();
}

/// <summary>
/// 更新処理。レンダリング対象のクリアと更新を行う。
/// </summary>
void ColliderRenderingSystem::Update() {
    // 描画するものがなかったらスキップ
    if (ShouldSkipRender()) {
        return;
    }

    // メッシュ作成
    CreateRenderMesh();

    // レンダリング
    Rendering();
}

/// <summary>
/// 終了処理
/// </summary>
void ColliderRenderingSystem::Finalize() {
    aabbRenderer_->Finalize();
    obbRenderer_->Finalize();
    sphereRenderer_->Finalize();
    rayRenderer_->Finalize();
    segmentRenderer_->Finalize();
    capsuleRenderer_->Finalize();

    dxCommand_->Finalize();
}

/// <summary>
/// コライダー描画用のパイプラインステートオブジェクト(PSO)を作成する
/// </summary>
void ColliderRenderingSystem::CreatePSO() {

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("ColoredVertex.VS");
    shaderManager->LoadShader("ColoredVertex.PS", kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo lineShaderInfo{};
    lineShaderInfo.vsKey = "ColoredVertex.VS";
    lineShaderInfo.psKey = "ColoredVertex.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[2]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    lineShaderInfo.pushBackRootParameter(rootParameter[0]);
    // CameraTransform ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[1].Descriptor.ShaderRegister = 1;
    lineShaderInfo.pushBackRootParameter(rootParameter[1]);
#pragma endregion

#pragma region "InputElement"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    lineShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "COLOR"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    lineShaderInfo.pushBackInputElementDesc(inputElementDesc);

#pragma endregion

    // topology
    lineShaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    // rasterizer
    lineShaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================
    pso_ = shaderManager->CreatePso("LineMesh_" + kBlendModeStr[int32_t(BlendMode::Alpha)], lineShaderInfo, dxDevice->device_);
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void ColliderRenderingSystem::StartRender() {
    auto& commandList = dxCommand_->GetCommandList();
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::GetInstance()->SetBufferForRootParameter(GetScene(), commandList, 1);
}

/// <summary>
/// 現在シーン内の全てのコライダーの形状に基づいて、ラインメッシュ情報を動的に生成する
/// </summary>
void ColliderRenderingSystem::CreateRenderMesh() {
    { // AABB
        auto& meshGroup = aabbRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        aabbMeshItr_ = meshGroup->begin();

        for (auto& slot : aabbColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = aabbColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue; // AABBColliderが存在しない場合はスキップ
            }
            for (auto& aabb : colliders) {

                if (!aabb.IsActive()) {
                    continue;
                }
                aabb.SetParent(transform);
                // 形状更新
                aabb.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                if (aabbMeshItr_->GetIndexCapacity() <= 0) {
                    aabbMeshItr_->TransferData();
                    ++aabbMeshItr_;
                    if (aabbMeshItr_ == meshGroup->end()) {
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kAabbVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kAabbIndexSize);
                        aabbMeshItr_ = --meshGroup->end();
                    }
                }

                // 色の設定
                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = aabb.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break; // 1つでも衝突していたら赤にする
                        }
                    }
                }

                // メッシュ作成
                CreateLineMesh(aabbMeshItr_._Ptr, aabb.GetWorldShape(), color);
            }
        }
    }
    aabbMeshItr_->TransferData();

    { // OBB
        auto& meshGroup = obbRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        obbMeshItr_ = meshGroup->begin();

        for (auto& slot : obbColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = obbColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue; // AABBColliderが存在しない場合はスキップ
            }
            for (auto& obb : colliders) {

                if (!obb.IsActive()) {
                    continue;
                }
                obb.SetParent(transform);
                // 形状更新
                obb.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                if (obbMeshItr_->GetIndexCapacity() <= 0) {
                    obbMeshItr_->TransferData();
                    ++obbMeshItr_;
                    if (obbMeshItr_ == meshGroup->end()) {
                        obbMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kObbVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kObbIndexSize);
                    }
                }

                // 色の設定
                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = obb.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break; // 1つでも衝突していたら赤にする
                        }
                    }
                }

                // メッシュ作成
                CreateLineMesh(obbMeshItr_._Ptr, obb.GetWorldShape(), color);
            }
        }
    }
    obbMeshItr_->TransferData();

    { // Sphere
        auto& meshGroup = sphereRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        sphereMeshItr_ = meshGroup->begin();

        for (auto& slot : sphereColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = sphereColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue; // sphereCollider が存在しない場合はスキップ
            }
            for (auto& sphere : colliders) {
                if (!sphere.IsActive()) {
                    continue;
                }
                sphere.SetParent(transform);
                // 形状更新
                sphere.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                if (sphereMeshItr_->GetIndexCapacity() <= 0) {
                    sphereMeshItr_->TransferData();
                    ++sphereMeshItr_;
                    if (sphereMeshItr_ == meshGroup->end()) {
                        sphereMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kSphereVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kSphereIndexSize);
                    }
                }

                // 色の設定
                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = sphere.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break; // 1つでも衝突していたら赤にする
                        }
                    }
                }
                // メッシュ作成
                CreateLineMesh(sphereMeshItr_._Ptr, sphere.GetWorldShape(), color);
            }
        }
    }
    sphereMeshItr_->TransferData();

    { // Ray
        auto& meshGroup = rayRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        rayMeshItr_ = meshGroup->begin();

        for (auto& slot : rayColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue;
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = rayColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue;
            }
            for (auto& ray : colliders) {
                if (!ray.IsActive()) {
                    continue;
                }
                ray.SetParent(transform);
                ray.CalculateWorldShape();

                if (rayMeshItr_->GetIndexCapacity() <= 0) {
                    rayMeshItr_->TransferData();
                    ++rayMeshItr_;
                    if (rayMeshItr_ == meshGroup->end()) {
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kRayVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kRayIndexSize);
                        rayMeshItr_ = --meshGroup->end();
                    }
                }

                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = ray.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break;
                        }
                    }
                }

                CreateLineMesh(rayMeshItr_._Ptr, ray.GetWorldShape(), color);
            }
        }
    }
    rayMeshItr_->TransferData();

    { // Segment
        auto& meshGroup = segmentRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        segmentMeshItr_ = meshGroup->begin();

        for (auto& slot : segmentColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue;
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = segmentColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue;
            }
            for (auto& segment : colliders) {
                if (!segment.IsActive()) {
                    continue;
                }
                segment.SetParent(transform);
                segment.CalculateWorldShape();

                if (segmentMeshItr_->GetIndexCapacity() <= 0) {
                    segmentMeshItr_->TransferData();
                    ++segmentMeshItr_;
                    if (segmentMeshItr_ == meshGroup->end()) {
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kSegmentVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kSegmentIndexSize);
                        segmentMeshItr_ = --meshGroup->end();
                    }
                }

                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = segment.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break;
                        }
                    }
                }

                CreateLineMesh(segmentMeshItr_._Ptr, segment.GetWorldShape(), color);
            }
        }
    }
    segmentMeshItr_->TransferData();

    { // Capsule
        auto& meshGroup = capsuleRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        capsuleMeshItr_ = meshGroup->begin();

        for (auto& slot : capsuleColliders_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue;
            }

            Transform* transform = GetComponent<Transform>(slot.owner);
            if (transform) {
                transform->UpdateMatrix();
            }

            auto& colliders = capsuleColliders_->GetComponents(slot.owner);
            if (colliders.empty()) {
                continue;
            }
            for (auto& capsule : colliders) {
                if (!capsule.IsActive()) {
                    continue;
                }
                capsule.SetParent(transform);
                capsule.CalculateWorldShape();

                if (capsuleMeshItr_->GetIndexCapacity() <= 0) {
                    capsuleMeshItr_->TransferData();
                    ++capsuleMeshItr_;
                    if (capsuleMeshItr_ == meshGroup->end()) {
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::kDefaultMeshCount_ * kCapsuleVertexSize, ColliderRenderingSystem::kDefaultMeshCount_ * kCapsuleIndexSize);
                        capsuleMeshItr_ = --meshGroup->end();
                    }
                }

                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = capsule.GetCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break;
                        }
                    }
                }

                CreateLineMesh(capsuleMeshItr_._Ptr, capsule.GetWorldShape(), color);
            }
        }
    }
    capsuleMeshItr_->TransferData();
}

/// <summary>
/// 描画コマンドの発行を行う
/// </summary>
void ColliderRenderingSystem::RenderCall() {
    auto& commandList = dxCommand_->GetCommandList();

    ///==============================
    /// 描画
    ///==============================
    aabbRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *aabbRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    obbRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *obbRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    sphereRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *sphereRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    rayRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *rayRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    segmentRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *segmentRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    capsuleRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *capsuleRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }
}

/// <summary>
/// コライダーのレンダリングを統合実行する
/// </summary>
void ColliderRenderingSystem::Rendering() {
    StartRender();

    RenderCall();
}

/// <summary>
/// レンダリングをスキップするかどうかを判定する
/// </summary>
/// <returns>true = 描画対象なし / false = 描画対象あり</returns>
bool ColliderRenderingSystem::ShouldSkipRender() const {
    bool isSkip = aabbColliders_->IsEmpty() && obbColliders_->IsEmpty() && sphereColliders_->IsEmpty() && rayColliders_->IsEmpty() && segmentColliders_->IsEmpty() && capsuleColliders_->IsEmpty();

    return isSkip;
}
