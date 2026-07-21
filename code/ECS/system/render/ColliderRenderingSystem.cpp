#include "ColliderRenderingSystem.h"
#include "ColliderMeshBuilder.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "asset/AssetSystem.h"

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
/// <remarks>
/// 形状(AABB/OBB/Sphere/Ray/Segment/Capsule)ごとにブロックが分かれているが、いずれも次の共通の流れで処理する。
/// ①前フレームで書き込んだ頂点・インデックスをクリアし、メッシュイテレータを先頭のメッシュへ戻す
/// ②コライダー配列の各スロットについて、所有EntityおよびTransformの有無を確認する
/// ③コライダーのローカル形状とTransformからワールド空間の形状を再計算する(CalculateWorldShape)
/// ④現在のメッシュの残りインデックス容量が尽きていれば、そのメッシュをTransferDataでGPUバッファへ確定転送し、
///   次のメッシュへ進む(無ければ新規にメッシュを確保する)
/// ⑤形状に応じた頂点・インデックスを追加する(CreateLineMesh)
/// </remarks>
void ColliderRenderingSystem::CreateRenderMesh() {
    { // AABB
        // AABB(軸平行境界ボックス)はMin/Maxのみで定義されるため、8頂点(kAabbVertexSize)を
        // ワールド座標のMin/Maxから直接生成し、辺を結ぶ12本の稜線(kAabbIndexSize=24、2点×12辺)を
        // ラインリストとして張る(CreateLineMesh内)。回転を持たないため頂点への行列適用は不要
        auto& meshGroup = aabbRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        aabbMeshItr_ = meshGroup->begin();

        for (auto& slot : aabbColliders_->GetSlots()) {
            // このシステムはentities_/EraseDeadEntityを経由せずComponentArrayのスロットを直接走査するため、
            // 所有Entityがすでに破棄されていないかをここで都度確認する必要がある
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            // Transformを持たないEntityも存在しうる(その場合SetParentにnullptrを渡し、
            // コライダーのローカル座標がそのままワールド座標として扱われる)
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げ、
                // 親子関係の変化(Transformの再取得含む)を毎フレーム反映させる
                aabb.SetParent(transform);
                // ローカル形状と(親子関係を反映した)Transformから、描画に使うワールド空間の形状を再計算する
                aabb.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                // (現在のメッシュに空きインデックスが無ければ、TransferDataでGPUバッファへ確定転送してから
                //  次のメッシュへ進む。無ければ新規にメッシュを確保する。1メッシュが持てる形状数には上限があるため)
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
        // OBB(有向境界ボックス)はローカルのhalfSize(半径)から8頂点(kObbVertexSize)を作り、
        // orientations_の回転とcenter_の平行移動をCreateLineMesh内で頂点に適用してから、
        // AABBと同じ12本の稜線(kObbIndexSize=24)をラインリストとして張る。回転がある分AABBと異なり
        // 頂点自体に行列を適用する必要がある
        auto& meshGroup = obbRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        obbMeshItr_ = meshGroup->begin();

        for (auto& slot : obbColliders_->GetSlots()) {
            // Entityがすでに破棄されている可能性があるため確認してからスキップする
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げる(毎フレーム再設定)
                obb.SetParent(transform);
                // ローカル形状とTransformから、描画に使うワールド空間の形状(回転込み)を再計算する
                obb.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                // (現在のメッシュに空きインデックスが無ければTransferDataで確定転送してから次のメッシュへ進む。
                //  無ければ新規にメッシュを確保する)
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
        // Sphereは緯度・経度をkSphereDivision(8)分割し、緯線(latIndex 1〜division-1周)と
        // 経線(lonIndex 0〜division-1周)をそれぞれ短い線分の集まりとして描画する。
        // AABB/OBBと異なり固定頂点配列は使わず、線分ごとに2頂点+2インデックスを動的に追加していく
        // (合計頂点・インデックス数はkSphereVertexSize/kSphereIndexSize = 4*division^2)
        auto& meshGroup = sphereRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        sphereMeshItr_ = meshGroup->begin();

        for (auto& slot : sphereColliders_->GetSlots()) {
            // Entityがすでに破棄されている可能性があるため確認してからスキップする
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げる(毎フレーム再設定)
                sphere.SetParent(transform);
                // ローカル形状とTransformから、描画に使うワールド空間の形状を再計算する
                sphere.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                // (現在のメッシュに空きインデックスが無ければTransferDataで確定転送してから次のメッシュへ進む。
                //  無ければ新規にメッシュを確保する)
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
        // Rayはorigin(始点)とdirection(向き)のみで終点を持たないため、CreateLineMesh側で
        // origin + direction * kRayLength(=100) を終点とみなし、1本の線分(kRayVertexSize/kRayIndexSize=2)として描画する
        auto& meshGroup = rayRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        rayMeshItr_ = meshGroup->begin();

        for (auto& slot : rayColliders_->GetSlots()) {
            // Entityがすでに破棄されている可能性があるため確認してからスキップする
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げる(毎フレーム再設定)
                ray.SetParent(transform);
                // ローカル形状とTransformから、描画に使うワールド空間の形状を再計算する
                ray.CalculateWorldShape();

                // 現在のメッシュに空きインデックスが無ければTransferDataで確定転送してから次のメッシュへ進む。
                // 無ければ新規にメッシュを確保する
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
        // Segmentはstart/endの2点で完結する形状のため、そのまま1本の線分(kSegmentVertexSize/IndexSize=2)として描画する
        auto& meshGroup = segmentRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        segmentMeshItr_ = meshGroup->begin();

        for (auto& slot : segmentColliders_->GetSlots()) {
            // Entityがすでに破棄されている可能性があるため確認してからスキップする
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げる(毎フレーム再設定)
                segment.SetParent(transform);
                // ローカル形状とTransformから、描画に使うワールド空間の形状を再計算する
                segment.CalculateWorldShape();

                // 現在のメッシュに空きインデックスが無ければTransferDataで確定転送してから次のメッシュへ進む。
                // 無ければ新規にメッシュを確保する
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
        // Capsuleは中心軸の線分(start-end、2頂点)に加え、始点側・終点側それぞれの円周
        // (kCapsuleDivision分割のリング)、さらに両リングを結ぶ4本の縦ラインで円柱状の輪郭を表現する
        // (合計kCapsuleVertexSize/IndexSize = 2 + 4*division*2)。軸長がほぼ0の場合は
        // CreateLineMesh側でSphereとして描画される(始点と終点が重なりカプセルとして成立しないため)
        auto& meshGroup = capsuleRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        capsuleMeshItr_ = meshGroup->begin();

        for (auto& slot : capsuleColliders_->GetSlots()) {
            // Entityがすでに破棄されている可能性があるため確認してからスキップする
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
                    continue; // 無効化中のコライダーは描画対象から除外する
                }
                // コライダー内部のTransformを所有EntityのTransformにぶら下げる(毎フレーム再設定)
                capsule.SetParent(transform);
                // ローカル形状とTransformから、描画に使うワールド空間の形状を再計算する
                capsule.CalculateWorldShape();

                // 現在のメッシュに空きインデックスが無ければTransferDataで確定転送してから次のメッシュへ進む。
                // 無ければ新規にメッシュを確保する
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
