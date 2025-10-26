#include "SkeletonRenderSystem.h"

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
#include "component/transform/Transform.h"

/// math
#include <numbers>

SkeletonRenderSystem::SkeletonRenderSystem()
    : ISystem(SystemCategory::Render) {}

SkeletonRenderSystem::~SkeletonRenderSystem() {}

const int32_t SkeletonRenderSystem::defaultMeshCount_ = 1000;

//** Sphere **//
static const uint32_t jointSphereDivision  = 8;
static const float jointSphereDivisionReal = static_cast<float>(jointSphereDivision);

static const uint32_t jointSphereVertexSize = 4 * jointSphereDivision * jointSphereDivision;
static const uint32_t jointSphereIndexSize  = 4 * jointSphereDivision * jointSphereDivision;

void SkeletonRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    //** ModelMeshRenderer **//
    skinningAnimationArray_ = getComponentArray<SkinningAnimationComponent>();

    //** JointMeshRenderer **//
    jointRenderer_ = LineRenderer(std::vector<Mesh<ColorVertexData>>());
    jointRenderer_.Initialize(nullptr);
    jointRenderer_.getMeshGroup()->push_back(Mesh<ColorVertexData>());
    jointRenderer_.getMeshGroup()->back().Initialize(SkeletonRenderSystem::defaultMeshCount_ * jointSphereVertexSize, SkeletonRenderSystem::defaultMeshCount_ * jointSphereIndexSize);
    jointMeshItr_ = jointRenderer_.getMeshGroup()->begin();

    //** BoneMeshRenderer **//
    boneRenderer_ = LineRenderer(std::vector<Mesh<ColorVertexData>>());
    boneRenderer_.Initialize(nullptr);
    boneRenderer_.getMeshGroup()->push_back(Mesh<ColorVertexData>());
    boneRenderer_.getMeshGroup()->back().Initialize(SkeletonRenderSystem::defaultMeshCount_ * 2, SkeletonRenderSystem::defaultMeshCount_ * 2);
    boneMeshItr_ = boneRenderer_.getMeshGroup()->begin();

    CreatePso();
}

void SkeletonRenderSystem::Update() {
    StartRender();

    CreateRenderMesh();

    RenderCall();
}

void SkeletonRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void SkeletonRenderSystem::CreateRenderMesh() {
    auto& jointMeshGroup = jointRenderer_.getMeshGroup();
    auto& boneMeshGroup  = boneRenderer_.getMeshGroup();

    // Meshの初期化
    for (auto meshItr = jointMeshGroup->begin(); meshItr != jointMeshGroup->end(); ++meshItr) {
        meshItr->vertexes_.clear();
        meshItr->indexes_.clear();
    }
    for (auto meshItr = boneMeshGroup->begin(); meshItr != boneMeshGroup->end(); ++meshItr) {
        meshItr->vertexes_.clear();
        meshItr->indexes_.clear();
    }

    // Meshのイテレータ初期化
    jointMeshItr_ = jointMeshGroup->begin();
    boneMeshItr_  = boneMeshGroup->begin();

    for (auto& [entityIdx, modelMeshIdx] : skinningAnimationArray_->getEntityIndexBind()) {
        Entity* entity = getEntity(entityIdx);
        if (!entity) {
            continue; // Entityが存在しない場合はスキップ
        }

        Matrix4x4 worldMat = MakeMatrix::Identity();
        if (auto* transform = getComponent<Transform>(entity); transform != nullptr) {
            worldMat = transform->worldMat;
        }

        auto skinningAnimationComps = skinningAnimationArray_->getComponents(entity);
        if (!skinningAnimationComps) {
            continue; // modelMeshRendererが存在しない場合はスキップ
        }

        for (auto& skinningAnimationComp : *skinningAnimationComps) {
            const auto& skeleton = skinningAnimationComp.getSkeleton();

            if (skeleton.joints.empty()) {
                continue;
            }

            CreateMeshForChildren(
                jointMeshGroup.get(),
                boneMeshGroup.get(),
                worldMat,
                skeleton,
                skeleton.joints[skeleton.rootJointIndex],
                nullptr,
                {0.f, 0.f, 0.f});
        }
    }

    jointMeshItr_->TransferData();
    boneMeshItr_->TransferData();
}

void SkeletonRenderSystem::RenderCall() {
    auto& commandList = dxCommand_->getCommandList();

    ///==============================
    /// 描画
    ///==============================
    jointRenderer_.getTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *jointRenderer_.getMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    boneRenderer_.getTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *boneRenderer_.getMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }
}

void SkeletonRenderSystem::CreatePso() {

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    ///=================================================
    /// 作成されているかチェック
    ///=================================================
    bool isLoaded = true;
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        pso_[blend]     = shaderManager->getPipelineStateObj("LineMesh_" + blendModeStr[i]);

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

    lineShaderInfo.customDepthStencilDesc().DepthEnable = false;

    // topology
    lineShaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    // rasterizer
    lineShaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        lineShaderInfo.blendMode_       = blend;
        pso_[lineShaderInfo.blendMode_] = shaderManager->CreatePso("LineMesh_" + blendModeStr[i], lineShaderInfo, dxDevice->device_);
    }
}

void SkeletonRenderSystem::StartRender() {
    currentBlend_ = BlendMode::Alpha;

    auto& commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}

void SkeletonRenderSystem::CreateMeshForChildren(
    std::vector<Mesh<ColorVertexData>>* _jointMeshGroup,
    std::vector<Mesh<ColorVertexData>>* _boneMeshGroup,
    const Matrix4x4& _worldMat,
    const Skeleton& _skeleton,
    const Joint& _joint,
    const Joint* _prevJoint,
    const Vec3f& _prevJointPos) {

    // Capacityが足りなかったら 新しいMeshを作成する
    if (jointMeshItr_->getIndexCapacity() <= 0) {
        jointMeshItr_->TransferData();
        ++jointMeshItr_;
        if (jointMeshItr_ == _jointMeshGroup->end()) {
            jointMeshItr_ = _jointMeshGroup->end();
            _jointMeshGroup->push_back(Mesh<ColorVertexData>());
            _jointMeshGroup->back().Initialize(SkeletonRenderSystem::defaultMeshCount_ * jointSphereDivision, SkeletonRenderSystem::defaultMeshCount_ * jointSphereDivision);
        }
    }
    if (boneMeshItr_->getIndexCapacity() <= 0) {
        boneMeshItr_->TransferData();
        ++boneMeshItr_;
        if (boneMeshItr_ == _boneMeshGroup->end()) {
            boneMeshItr_ = _boneMeshGroup->end();
            _boneMeshGroup->push_back(Mesh<ColorVertexData>());
            _boneMeshGroup->back().Initialize(SkeletonRenderSystem::defaultMeshCount_ * 2, SkeletonRenderSystem::defaultMeshCount_ * 2);
        }
    }

    Vec3f jointCenter = (_joint.skeletonSpaceMatrix * _worldMat)[3];
    // メッシュ作成
    CreateJointMesh(jointMeshItr_._Ptr, _joint, jointCenter, {1.f, 1.f, 1.f, 1.f});
    // ボーンメッシュ作成
    if (_prevJoint) {
        CreateBoneMesh(boneMeshItr_._Ptr, _prevJointPos, jointCenter, {1.f, 1.f, 1.f, 1.f});
    }
    _prevJoint = &_joint;

    // 子のジョイントに対して再帰的に処理を行う
    for (const auto& childIndex : _joint.children) {
        CreateMeshForChildren(
            _jointMeshGroup, _boneMeshGroup,
            _worldMat,
            _skeleton,
            _skeleton.joints[childIndex],
            &_joint, jointCenter);
    }
}

void SkeletonRenderSystem::CreateJointMesh(
    Mesh<ColorVertexData>* _mesh,
    const Joint& _joint,
    const Vec3f& _center,
    const Vec4f& _color) {

    const float kLatEvery = std::numbers::pi_v<float> / jointSphereDivisionReal; //* 緯度
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / jointSphereDivisionReal; //* 経度

    auto calculatePoint = [&](float lat, float lon) -> Vector3f {
        return {
            _center[X] + (_joint.transform.scale[X] * 0.01f) * std::cos(lat) * std::cos(lon),
            _center[Y] + (_joint.transform.scale[Y] * 0.01f) * std::sin(lat),
            _center[Z] + (_joint.transform.scale[Z] * 0.01f) * std::cos(lat) * std::sin(lon)};
    };

    // 緯線（緯度方向の円）を描画
    for (uint32_t latIndex = 1; latIndex < jointSphereDivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < jointSphereDivision; ++lonIndex) {
            float lonA = lonIndex * kLonEvery;
            float lonB = (lonIndex + 1) % jointSphereDivision * kLonEvery;

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
    for (uint32_t lonIndex = 0; lonIndex < jointSphereDivision; ++lonIndex) {
        float lon = lonIndex * kLonEvery;
        for (uint32_t latIndex = 0; latIndex < jointSphereDivision; ++latIndex) {
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

void SkeletonRenderSystem::CreateBoneMesh(
    Mesh<ColorVertexData>* _mesh,
    const Vec3f& _start,
    const Vec3f& _end,
    const Vec4f& _color) {
    _mesh->vertexes_.push_back(ColorVertexData{Vec4f(_start, 1.f), _color});
    _mesh->vertexes_.push_back(ColorVertexData{Vec4f(_end, 1.f), _color});

    _mesh->indexes_.push_back((uint32_t)_mesh->indexes_.size());
    _mesh->indexes_.push_back((uint32_t)_mesh->indexes_.size());
}
