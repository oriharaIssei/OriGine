#include "LightDebugRenderingSystem.h"

/// engine
#include "camera/CameraManager.h"
#include "EngineConfig.h"

/// math
#include "math/mathEnv.h"

using namespace OriGine;

const uint32_t LightDebugRenderingSystem::kDefaultMeshCount_ = 1000;

namespace {
//** Sphere **//
static constexpr uint32_t kSphereDivision  = 8;
static constexpr float kSphereDivisionReal = static_cast<float>(kSphereDivision);

static constexpr uint32_t kSphereVertexSize = 4 * kSphereDivision * kSphereDivision;
static constexpr uint32_t kSphereIndexSize  = 4 * kSphereDivision * kSphereDivision;

static constexpr uint32_t kSpotDivision   = 16;
static constexpr uint32_t kSpotVertexSize = (4 * kSpotDivision) + 2; // +2 は中央方向線分
static constexpr uint32_t kSpotIndexSize  = (4 * kSpotDivision) + 2;
}

#pragma region "CreateLightMesh"
template <typename ShapeType>
void CreateLineMeshByLightShape(
    Mesh<ColorVertexData>* _mesh,
    const ShapeType& _shape,
    const Vec4f& _color = kWhite) {
    _mesh;
    _shape;
}

template <>
void CreateLineMeshByLightShape(
    Mesh<ColorVertexData>* _mesh,
    const PointLight& _shape,
    const Vec4f& _color) {
    if (!_shape.isActive) {
        return;
    }

    const float kLatEvery = kPi / kSphereDivisionReal;
    const float kLonEvery = kTau / kSphereDivisionReal;

    auto calc = [&](float lat, float lon) -> Vec3f {
        return {
            _shape.pos[X] + _shape.radius * std::cos(lat) * std::cos(lon),
            _shape.pos[Y] + _shape.radius * std::sin(lat),
            _shape.pos[Z] + _shape.radius * std::cos(lat) * std::sin(lon)};
    };

    for (uint32_t lat = 1; lat < kSphereDivision; ++lat) {
        float latitude = -kPi / 2.0f + kLatEvery * lat;
        for (uint32_t lon = 0; lon < kSphereDivision; ++lon) {
            Vec3f a = calc(latitude, lon * kLonEvery);
            Vec3f b = calc(latitude, (lon + 1) * kLonEvery);

            uint32_t base = uint32_t(_mesh->vertexes_.size());
            _mesh->vertexes_.emplace_back(Vec4f(a, 1.f), _color);
            _mesh->vertexes_.emplace_back(Vec4f(b, 1.f), _color);
            _mesh->indexes_.push_back(base);
            _mesh->indexes_.push_back(base + 1);
        }
    }
}

template <>
void CreateLineMeshByLightShape(
    Mesh<ColorVertexData>* _mesh,
    const SpotLight& _light,
    const Vec4f& _color) {
    if (!_light.isActive) {
        return;
    }

    // ===== 基本情報 =====
    const Vec3f& apex = _light.pos;
    Vec3f dir         = Vec3f::Normalize(_light.direction);
    float range       = _light.distance;

    // cos → angle
    float outerAngle = std::acos(std::clamp(_light.cosAngle, -1.f, 1.f));

    // 円錐半径
    float outerRadius = std::tan(outerAngle) * range;

    // 先端中心
    Vec3f coneCenter = apex + dir * range;

    // ===== 円錐の基準軸を作る =====
    Vec3f right;
    if (std::abs(dir[Y]) < Config::Billboard::kThreshold) {
        right = Vec3f::Normalize(Vec3f::Cross(axisY, dir));
    } else {
        right = Vec3f::Normalize(Vec3f::Cross(axisX, dir));
    }
    Vec3f up = Vec3f::Normalize(Vec3f::Cross(dir, right));

    // ===== 外側コーン =====
    for (uint32_t i = 0; i < kSpotDivision; ++i) {
        float a0 = (i / float(kSpotDivision)) * kTau;
        float a1 = ((i + 1) / float(kSpotDivision)) * kTau;

        Vec3f p0 = coneCenter + (right * std::cos(a0) + up * std::sin(a0)) * outerRadius;
        Vec3f p1 = coneCenter + (right * std::cos(a1) + up * std::sin(a1)) * outerRadius;

        uint32_t base = uint32_t(_mesh->vertexes_.size());

        // 円周
        _mesh->vertexes_.emplace_back(Vec4f(p0, 1.f), _color);
        _mesh->vertexes_.emplace_back(Vec4f(p1, 1.f), _color);
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);

        // 側面
        _mesh->vertexes_.emplace_back(Vec4f(apex, 1.f), _color);
        _mesh->vertexes_.emplace_back(Vec4f(p0, 1.f), _color);
        _mesh->indexes_.push_back(base + 2);
        _mesh->indexes_.push_back(base + 3);
    }

    // ===== 中央方向線 =====
    constexpr Vec4f kCenterLineColor = Vec4f(1.f, 0.89f, 0.35f, 1.f);
    {
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(Vec4f(apex, 1.f), kCenterLineColor);
        _mesh->vertexes_.emplace_back(Vec4f(coneCenter, 1.f), kCenterLineColor);
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }
}

#pragma endregion

LightDebugRenderingSystem::LightDebugRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
LightDebugRenderingSystem::~LightDebugRenderingSystem() {}

/// <summary>
/// 初期化処理。ポイントライトとスポットライト用のレンダラー生成を行う。
/// </summary>
void LightDebugRenderingSystem::Initialize() {
    BaseRenderSystem::Initialize();

    pointLights_ = GetComponentArray<PointLight>();
    spotLights_  = GetComponentArray<SpotLight>();

    pointRenderer_ = std::make_unique<LineRenderer>();
    pointRenderer_->Initialize(nullptr, EntityHandle());
    pointRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    pointRenderer_->GetMeshGroup()->back().Initialize(
        LightDebugRenderingSystem::kDefaultMeshCount_ * kSphereVertexSize,
        LightDebugRenderingSystem::kDefaultMeshCount_ * kSphereVertexSize);
    pointLightMeshItr_ = pointRenderer_->GetMeshGroup()->begin();

    spotRenderer_ = std::make_unique<LineRenderer>();
    spotRenderer_->Initialize(nullptr, EntityHandle());
    spotRenderer_->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
    spotRenderer_->GetMeshGroup()->back().Initialize(LightDebugRenderingSystem::kDefaultMeshCount_ * kSpotVertexSize, LightDebugRenderingSystem::kDefaultMeshCount_ * kSpotIndexSize);
    spotLightMeshItr_ = spotRenderer_->GetMeshGroup()->begin();
    CreateRenderMesh();
    CreatePSO();
}

/// <summary>
/// 更新処理。レンダリング対象のクリアと更新を行う。
/// </summary>
void LightDebugRenderingSystem::Update() {
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
void LightDebugRenderingSystem::Finalize() {
    pointRenderer_->Finalize();
    spotRenderer_->Finalize();
    dxCommand_->Finalize();
}

/// <summary>
/// ライトデバッグ情報のレンダリングを統合実行する
/// </summary>
void LightDebugRenderingSystem::Rendering() {
    StartRender();
    RenderCall();
}

/// <summary>
/// レンダリングをスキップするかどうかを判定する
/// </summary>
/// <returns>true = 描画対象なし / false = 描画対象あり</returns>
bool LightDebugRenderingSystem::ShouldSkipRender() const {
    return pointLights_->IsEmpty() && spotLights_->IsEmpty();
}

/// <summary>
/// ライトデバッグ描画用のパイプラインステートオブジェクト(PSO)を作成する
/// </summary>
void LightDebugRenderingSystem::CreatePSO() {

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
/// レンダリング開始処理。ビュー情報の設定などを行う。
/// </summary>
void LightDebugRenderingSystem::StartRender() {
    auto& commandList = dxCommand_->GetCommandList();
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::GetInstance()->SetBufferForRootParameter(GetScene(), commandList, 1);
}

/// <summary>
/// 現在シーン内の全てのライトに基づき、デバッグ用ラインメッシュを動的に生成する
/// </summary>
void LightDebugRenderingSystem::CreateRenderMesh() {
    { // point
        auto& meshGroup = pointRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        pointLightMeshItr_ = meshGroup->begin();

        for (auto& slot : pointLights_->GetSlots()) {

            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            if (!slot.alive) {
                continue;
            }

            for (auto& spotlight : slot.components) {
                if (!spotlight.isActive) {
                    continue;
                }
                // Capacityが足りなかったら 新しいMeshを作成する
                if (pointLightMeshItr_->GetIndexCapacity() <= 0) {
                    pointLightMeshItr_->TransferData();
                    ++pointLightMeshItr_;
                    if (pointLightMeshItr_ == meshGroup->end()) {
                        pointLightMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(LightDebugRenderingSystem::kDefaultMeshCount_ * kSphereVertexSize, LightDebugRenderingSystem::kDefaultMeshCount_ * kSphereIndexSize);
                    }
                }

                // メッシュ作成
                CreateLineMeshByLightShape<>(pointLightMeshItr_._Ptr, spotlight, kWhite);
            }
        }
    }
    pointLightMeshItr_->TransferData();

    { // spot
        auto& meshGroup = spotRenderer_->GetMeshGroup();

        for (auto meshItr = meshGroup->begin(); meshItr != meshGroup->end(); ++meshItr) {
            meshItr->vertexes_.clear();
            meshItr->indexes_.clear();
        }

        spotLightMeshItr_ = meshGroup->begin();

        for (auto& slot : spotLights_->GetSlots()) {
            Entity* entity = GetEntity(slot.owner);
            if (!entity) {
                continue; // Entityが存在しない場合はスキップ
            }

            if (!slot.alive) {
                continue;
            }

            for (auto& spotlight : slot.components) {
                if (!spotlight.isActive) {
                    continue;
                }

                // Capacityが足りなかったら 新しいMeshを作成する
                if (spotLightMeshItr_->GetIndexCapacity() <= 0) {
                    spotLightMeshItr_->TransferData();
                    ++spotLightMeshItr_;
                    if (spotLightMeshItr_ == meshGroup->end()) {
                        spotLightMeshItr_ = meshGroup->end();
                        meshGroup->push_back(Mesh<ColorVertexData>());
                        meshGroup->back().Initialize(LightDebugRenderingSystem::kDefaultMeshCount_ * kSpotVertexSize, LightDebugRenderingSystem::kDefaultMeshCount_ * kSpotIndexSize);
                    }
                }

                // メッシュ作成
                CreateLineMeshByLightShape<>(spotLightMeshItr_._Ptr, spotlight);
            }
        }
    }
    spotLightMeshItr_->TransferData();
}

/// <summary>
/// 描画コマンドの発行を行う
/// </summary>
void LightDebugRenderingSystem::RenderCall() {
    auto& commandList = dxCommand_->GetCommandList();

    ///==============================
    /// 描画
    ///==============================
    pointRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *pointRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }

    spotRenderer_->GetTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *spotRenderer_->GetMeshGroup()) {
        if (mesh.indexes_.size() <= 0) {
            continue;
        }
        // 描画
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
    }
}
