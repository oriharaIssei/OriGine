#include "ColliderRenderingSystem.h"

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

const int32_t ColliderRenderingSystem::kDefaultMeshCount_ = 1000;
namespace {
// ** AABB **//
static const uint32_t kAabbVertexSize = 8;
static const uint32_t kAabbIndexSize  = 24;

//** OBB **//
static const uint32_t kObbVertexSize = 8;
static const uint32_t kObbIndexSize  = 24;

//** Sphere **//
static const uint32_t kSphereDivision  = 8;
static const float kSphereDivisionReal = static_cast<float>(kSphereDivision);

static const uint32_t kSphereVertexSize = 4 * kSphereDivision * kSphereDivision;
static const uint32_t kSphereIndexSize  = 4 * kSphereDivision * kSphereDivision;

//** Ray **//
static const float kRayLength        = 100.0f; // 描画用の仮想長さ
static const uint32_t kRayVertexSize = 2;
static const uint32_t kRayIndexSize  = 2;

//** Segment **//
static const uint32_t kSegmentVertexSize = 2;
static const uint32_t kSegmentIndexSize  = 2;

//** Capsule **//
static const uint32_t kCapsuleDivision   = 8;
static const float kCapsuleDivisionReal  = static_cast<float>(kCapsuleDivision);
static const uint32_t kCapsuleVertexSize = 2 + 4 * kCapsuleDivision * 2; // 軸 + 両端の球
static const uint32_t kCapsuleIndexSize  = 2 + 4 * kCapsuleDivision * 2;
} // namespace

#pragma region "CreateLineMesh"
/// <summary>
/// Bounds形状からラインメッシュを作成
/// </summary>
/// <typeparam name="ShapeType">形状クラス</typeparam>
/// <param name="_mesh">出力先</param>
/// <param name="_shape">形状情報</param>
/// <param name="_color">メッシュの色</param>
template <Bounds::IsBounds ShapeType>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const ShapeType& _shape,
    const Vec4f& _color = kWhite) {
    _mesh;
    _shape;
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::AABB& _shape,
    const Vec4f& _color) {
    Vec3f shapeMin = _shape.Min();
    Vec3f shapeMax = _shape.Max();

    // AABBVertex
    Vector3f vertexes[kAabbVertexSize]{
        {shapeMin},
        {shapeMin[X], shapeMin[Y], shapeMax[Z]},
        {shapeMax[X], shapeMin[Y], shapeMax[Z]},
        {shapeMax[X], shapeMin[Y], shapeMin[Z]},
        {shapeMin[X], shapeMax[Y], shapeMin[Z]},
        {shapeMin[X], shapeMax[Y], shapeMax[Z]},
        {shapeMax},
        {shapeMax[X], shapeMax[Y], shapeMin[Z]}};

    // AABBIndex
    uint32_t indices[kAabbIndexSize]{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7};

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    // 頂点バッファにデータを格納
    for (uint32_t vi = 0; vi < kAabbVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(vertexes[vi], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < kAabbIndexSize; ++ii) {
        _mesh->indexes_.emplace_back(startIndexesIndex + indices[ii]);
    }
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::OBB& _shape,
    const Vec4f& _color) {
    // OBBの8頂点を計算
    Vector3f halfSizes  = _shape.halfSize_;
    Vector3f corners[8] = {
        {-halfSizes[X], -halfSizes[Y], -halfSizes[Z]},
        {halfSizes[X], -halfSizes[Y], -halfSizes[Z]},
        {halfSizes[X], halfSizes[Y], -halfSizes[Z]},
        {-halfSizes[X], halfSizes[Y], -halfSizes[Z]},
        {-halfSizes[X], -halfSizes[Y], halfSizes[Z]},
        {halfSizes[X], -halfSizes[Y], halfSizes[Z]},
        {halfSizes[X], halfSizes[Y], halfSizes[Z]},
        {-halfSizes[X], halfSizes[Y], halfSizes[Z]}};

    // 回転と位置を適用
    Matrix4x4 rotationMatrix = MakeMatrix4x4::RotateQuaternion(_shape.orientations_.rot);
    for (auto& corner : corners) {
        corner = corner * rotationMatrix + _shape.center_;
    }
    // OBBIndex
    uint32_t indices[kObbIndexSize]{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7};
    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());
    // 頂点バッファにデータを格納
    for (uint32_t vi = 0; vi < kObbVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(corners[vi], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < kObbIndexSize; ++ii) {
        _mesh->indexes_.emplace_back(startIndexesIndex + indices[ii]);
    }
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Sphere& _shape,
    const Vec4f& _color) {

    const float kLatEvery = std::numbers::pi_v<float> / kSphereDivisionReal; //* 緯度
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / kSphereDivisionReal; //* 経度

    auto calculatePoint = [&](float lat, float lon) -> Vector3f {
        return {
            _shape.center_[X] + _shape.radius_ * std::cos(lat) * std::cos(lon),
            _shape.center_[Y] + _shape.radius_ * std::sin(lat),
            _shape.center_[Z] + _shape.radius_ * std::cos(lat) * std::sin(lon)};
    };

    // 緯線（緯度方向の円）を描画
    for (uint32_t latIndex = 1; latIndex < kSphereDivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < kSphereDivision; ++lonIndex) {
            float lonA = lonIndex * kLonEvery;
            float lonB = (lonIndex + 1) % kSphereDivision * kLonEvery;

            Vector3f pointA = calculatePoint(lat, lonA);
            Vector3f pointB = calculatePoint(lat, lonB);

            // 頂点バッファにデータを格納
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointA, 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointB, 1.f), _color});

            // インデックスバッファにデータを格納
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        }
    }

    // 経線（経度方向の円）を描画
    for (uint32_t lonIndex = 0; lonIndex < kSphereDivision; ++lonIndex) {
        float lon = lonIndex * kLonEvery;
        for (uint32_t latIndex = 0; latIndex < kSphereDivision; ++latIndex) {
            float latA = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
            float latB = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

            Vector3f pointA = calculatePoint(latA, lon);
            Vector3f pointB = calculatePoint(latB, lon);

            // 頂点バッファにデータを格納
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointA, 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointB, 1.f), _color});

            // インデックスバッファにデータを格納
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        }
    }
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Ray& _shape,
    const Vec4f& _color) {
    // Rayは始点から方向に向かって一定の長さまで描画
    Vector3f origin   = _shape.origin;
    Vector3f endPoint = origin + _shape.direction * kRayLength;

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    // 頂点バッファにデータを格納
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(origin, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(endPoint, 1.f), _color});

    // インデックスバッファにデータを格納
    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Segment& _shape,
    const Vec4f& _color) {
    Vector3f start = _shape.start;
    Vector3f end   = _shape.end;

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    // 頂点バッファにデータを格納
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(start, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(end, 1.f), _color});

    // インデックスバッファにデータを格納
    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Capsule& _shape,
    const Vec4f& _color) {
    Vector3f start = _shape.segment.start;
    Vector3f end   = _shape.segment.end;
    float radius   = _shape.radius;

    // カプセルの軸方向
    Vector3f axis    = Vec3f(end - start);
    float axisLength = axis.length();

    // 軸が0の場合は球として描画
    if (axisLength < 0.0001f) {
        Bounds::Sphere sphere;
        sphere.center_ = start;
        sphere.radius_ = radius;
        CreateLineMeshByShape(_mesh, sphere, _color);
        return;
    }

    Vector3f axisNorm = axis.normalize();

    // 軸に垂直なベクトルを求める
    Vector3f perpendicular;
    if (std::abs(axisNorm[Y]) < 0.99f) {
        perpendicular = Vec3f::Cross({0.f, 1.f, 0.f}, axisNorm).normalize();
    } else {
        perpendicular = Vec3f::Cross({1.f, 0.f, 0.f}, axisNorm).normalize();
    }
    Vector3f perpendicular2 = Vec3f::Cross(axisNorm, perpendicular).normalize();

    const float kAngleStep = 2.0f * std::numbers::pi_v<float> / kCapsuleDivisionReal;

    // 中心軸を描画
    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(start, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(end, 1.f), _color});
    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);

    // 始点側の円を描画
    for (uint32_t i = 0; i < kCapsuleDivision; ++i) {
        float angle1 = i * kAngleStep;
        float angle2 = (i + 1) % kCapsuleDivision * kAngleStep;

        Vector3f p1 = start + perpendicular * (radius * std::cos(angle1)) + perpendicular2 * (radius * std::sin(angle1));
        Vector3f p2 = start + perpendicular * (radius * std::cos(angle2)) + perpendicular2 * (radius * std::sin(angle2));

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }

    // 終点側の円を描画
    for (uint32_t i = 0; i < kCapsuleDivision; ++i) {
        float angle1 = i * kAngleStep;
        float angle2 = (i + 1) % kCapsuleDivision * kAngleStep;

        Vector3f p1 = end + perpendicular * (radius * std::cos(angle1)) + perpendicular2 * (radius * std::sin(angle1));
        Vector3f p2 = end + perpendicular * (radius * std::cos(angle2)) + perpendicular2 * (radius * std::sin(angle2));

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }

    // 両端を繋ぐ縦のラインを描画（4本）
    for (uint32_t i = 0; i < 4; ++i) {
        float angle     = i * (std::numbers::pi_v<float> / 2.0f);
        Vector3f offset = perpendicular * (radius * std::cos(angle)) + perpendicular2 * (radius * std::sin(angle));
        Vector3f p1     = start + offset;
        Vector3f p2     = end + offset;

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }
}
#pragma endregion

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
                CreateLineMeshByShape<>(aabbMeshItr_._Ptr, aabb.GetWorldShape(), color);
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
                CreateLineMeshByShape<>(obbMeshItr_._Ptr, obb.GetWorldShape(), color);
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
                CreateLineMeshByShape<>(sphereMeshItr_._Ptr, sphere.GetWorldShape(), color);
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

                CreateLineMeshByShape<>(rayMeshItr_._Ptr, ray.GetWorldShape(), color);
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

                CreateLineMeshByShape<>(segmentMeshItr_._Ptr, segment.GetWorldShape(), color);
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

                CreateLineMeshByShape<>(capsuleMeshItr_._Ptr, capsule.GetWorldShape(), color);
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
