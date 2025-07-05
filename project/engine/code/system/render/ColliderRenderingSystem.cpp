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
#include <numbers>

const int32_t ColliderRenderingSystem::defaultMeshCount_ = 1000;

// ** AABB **//
static const uint32_t aabbVertexSize = 8;
static const uint32_t aabbIndexSize  = 24;

//** Sphere **//
static const uint32_t sphereDivision  = 8;
static const float sphereDivisionReal = static_cast<float>(sphereDivision);

static const uint32_t sphereVertexSize = 4 * sphereDivision * sphereDivision;
static const uint32_t sphereIndexSize  = 4 * sphereDivision * sphereDivision;

void ColliderRenderingSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    //** AABB **//
    aabbColliders_ = getComponentArray<AABBCollider>();
    aabbRenderer_  = LineRenderer(std::vector<Mesh<ColorVertexData>>());
    aabbRenderer_.Initialize(nullptr);
    aabbRenderer_.getMeshGroup()->push_back(Mesh<ColorVertexData>());
    aabbRenderer_.getMeshGroup()->back().Initialize(ColliderRenderingSystem::defaultMeshCount_ * aabbVertexSize, ColliderRenderingSystem::defaultMeshCount_ * aabbIndexSize);
    aabbMeshItr_ = aabbRenderer_.getMeshGroup()->begin();

    //** Sphere **//
    sphereColliders_ = getComponentArray<SphereCollider>();
    sphereRenderer_  = LineRenderer(std::vector<Mesh<ColorVertexData>>());
    sphereRenderer_.Initialize(nullptr);
    sphereRenderer_.getMeshGroup()->push_back(Mesh<ColorVertexData>());
    sphereRenderer_.getMeshGroup()->back().Initialize(ColliderRenderingSystem::defaultMeshCount_ * sphereVertexSize, ColliderRenderingSystem::defaultMeshCount_ * sphereIndexSize);
    sphereMeshItr_ = sphereRenderer_.getMeshGroup()->begin();

    CreatePso();
}

#pragma region "CreateLineMesh"
template <IsShape ShapeType>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const ShapeType& _shape,
    const Vec4f& _color = {1.f, 1.f, 1.f, 1.f}) {
    _mesh;
    _shape;
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const AABB& _shape,
    const Vec4f& _color) {

    // AABBVertex
    Vector3f vertexes[aabbVertexSize]{
        {_shape.min_},
        {_shape.min_[X], _shape.min_[Y], _shape.max_[Z]},
        {_shape.max_[X], _shape.min_[Y], _shape.max_[Z]},
        {_shape.max_[X], _shape.min_[Y], _shape.min_[Z]},
        {_shape.min_[X], _shape.max_[Y], _shape.min_[Z]},
        {_shape.min_[X], _shape.max_[Y], _shape.max_[Z]},
        {_shape.max_},
        {_shape.max_[X], _shape.max_[Y], _shape.min_[Z]}};

    // AABBIndex
    uint32_t indices[aabbIndexSize]{
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
    for (uint32_t vi = 0; vi < aabbVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(vertexes[vi], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < aabbIndexSize; ++ii) {
        _mesh->indexes_.emplace_back(startIndexesIndex + indices[ii]);
    }
}

template <>
void CreateLineMeshByShape(
    Mesh<ColorVertexData>* _mesh,
    const Sphere& _shape,
    const Vec4f& _color) {

    const float kLatEvery = std::numbers::pi_v<float> / sphereDivisionReal; //* 緯度
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / sphereDivisionReal; //* 経度

    auto calculatePoint = [&](float lat, float lon) -> Vector3f {
        return {
            _shape.center_[X] + _shape.radius_ * std::cos(lat) * std::cos(lon),
            _shape.center_[Y] + _shape.radius_ * std::sin(lat),
            _shape.center_[Z] + _shape.radius_ * std::cos(lat) * std::sin(lon)};
    };

    // 緯線（緯度方向の円）を描画
    for (uint32_t latIndex = 1; latIndex < sphereDivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < sphereDivision; ++lonIndex) {
            float lonA = lonIndex * kLonEvery;
            float lonB = (lonIndex + 1) % sphereDivision * kLonEvery;

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
    for (uint32_t lonIndex = 0; lonIndex < sphereDivision; ++lonIndex) {
        float lon = lonIndex * kLonEvery;
        for (uint32_t latIndex = 0; latIndex < sphereDivision; ++latIndex) {
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
#pragma endregion

void ColliderRenderingSystem::Update() {
    StartRender();

    CreateRenderMesh();

    RenderCall();
}

void ColliderRenderingSystem::Finalize() {
    dxCommand_->Finalize();
}

void ColliderRenderingSystem::UpdateEntity(GameEntity* /*_entity*/) {}

void ColliderRenderingSystem::CreateRenderMesh() {
    { // AABB
        auto& meshGroup = aabbRenderer_.getMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        aabbMeshItr_ = meshGroup->begin();

        for (auto& [entityIdx, aabbIdx] : aabbColliders_->getEntityIndexBind()) {
            GameEntity* entity = getEntity(entityIdx);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            Transform* transform = getComponent<Transform>(entity);
            if (transform) {
                transform->Update();
            }

            auto colliders = aabbColliders_->getComponents(entity);
            if (!colliders) {
                continue; // AABBColliderが存在しない場合はスキップ
            }
            for (auto& aabb : *colliders) {
                
                if (!aabb.isActive()) {
                    continue;
                }
                aabb.setParent(transform);
                // 形状更新
                aabb.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                if (aabbMeshItr_->getIndexCapacity() <= 0) {
                    aabbMeshItr_->TransferData();
                    ++aabbMeshItr_;
                    if (aabbMeshItr_ == meshGroup->end()) {
                        aabbMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::defaultMeshCount_ * aabbVertexSize, ColliderRenderingSystem::defaultMeshCount_ * aabbIndexSize);
                    }
                }

                // 色の設定
                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = aabb.getCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break; // 1つでも衝突していたら赤にする
                        }
                    }
                }

                // メッシュ作成
                CreateLineMeshByShape<>(aabbMeshItr_._Ptr, aabb.getWorldShape(), color);
            }
        }
    }
    aabbMeshItr_->TransferData();

    { // Sphere
        auto& meshGroup = sphereRenderer_.getMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        sphereMeshItr_ = meshGroup->begin();

         for (auto& [entityIdx, sphereIdx] : sphereColliders_->getEntityIndexBind()) {
            GameEntity* entity = getEntity(entityIdx);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            Transform* transform = getComponent<Transform>(entity);
            if (transform) {
                transform->Update();
            }

            auto colliders = sphereColliders_->getComponents(entity);
            if (!colliders) {
                continue; // sphereCollider が存在しない場合はスキップ
            }
            for (auto& sphere : *colliders) {
                if (!sphere.isActive()) {
                    continue;
                }
                sphere.setParent(transform);
                // 形状更新
                sphere.CalculateWorldShape();

                // Capacityが足りなかったら 新しいMeshを作成する
                if (sphereMeshItr_->getIndexCapacity() <= 0) {
                    sphereMeshItr_->TransferData();
                    ++sphereMeshItr_;
                    if (sphereMeshItr_ == meshGroup->end()) {
                        sphereMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(ColliderRenderingSystem::defaultMeshCount_ * sphereVertexSize, ColliderRenderingSystem::defaultMeshCount_ * sphereIndexSize);
                    }
                }

                // 色の設定
                Vec4f color    = {1, 1, 1, 1};
                auto& stateMap = sphere.getCollisionStateMap();
                if (!stateMap.empty()) {
                    for (auto& [collEntityIdx, state] : stateMap) {
                        if (state != CollisionState::None) {
                            color = {1, 0, 0, 1};
                            break; // 1つでも衝突していたら赤にする
                        }
                    }
                }
                // メッシュ作成
                CreateLineMeshByShape<>(sphereMeshItr_._Ptr, sphere.getWorldShape(), color);
            }
        }
    }
    sphereMeshItr_->TransferData();
}

void ColliderRenderingSystem::RenderCall() {
    auto commandList = dxCommand_->getCommandList();

    ///==============================
    /// 描画
    ///==============================
    aabbRenderer_.getTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *aabbRenderer_.getMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    sphereRenderer_.getTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *sphereRenderer_.getMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }
}

void ColliderRenderingSystem::CreatePso() {

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    ///=================================================
    /// 作成されているかチェック
    ///=================================================
    bool isLoaded = true;
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        pso_[blend]     = shaderManager->getPipelineStateObj("LineMeshMesh_" + blendModeStr[i]);

        if (!pso_[blend]) {
            isLoaded = false;
        }
    }
    if (isLoaded) {
        return;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("ColoredVertex.VS");
    shaderManager->LoadShader("ColoredVertex.PS", shaderDirectory, L"ps_6_0");

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
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        if (pso_[blend]) {
            continue;
        }
        lineShaderInfo.blendMode_       = blend;
        pso_[lineShaderInfo.blendMode_] = shaderManager->CreatePso("LineMeshMesh_" + blendModeStr[i], lineShaderInfo, dxDevice->getDevice());
    }
}

void ColliderRenderingSystem::StartRender() {
    currentBlend_ = BlendMode::Alpha;

    auto commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}
