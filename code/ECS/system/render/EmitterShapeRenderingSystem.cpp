#include "EmitterShapeRenderingSystem.h"
#include "EmitterShapeMeshBuilder.h"

/// engine
#include "Engine.h"
#include "directX12/DxDevice.h"
#include "camera/CameraManager.h"

using namespace OriGine;
using namespace OriGine::EmitterShapeMesh;

const int32_t EmitterShapeRenderingSystem::kDefaultMeshCount_ = 256;

EmitterShapeRenderingSystem::EmitterShapeRenderingSystem()
    : BaseRenderSystem() {}

EmitterShapeRenderingSystem::~EmitterShapeRenderingSystem() {}

void EmitterShapeRenderingSystem::Initialize() {
    BaseRenderSystem::Initialize();

    emitters_ = GetComponentArray<ParticleSystem>();
    spawners_ = GetComponentArray<EntitySpawner>();

    auto makeRenderer = [](uint32_t _vertexSize, uint32_t _indexSize, int32_t _defaultCount) {
        auto renderer = std::make_unique<LineRenderer>(std::vector<Mesh<ColorVertexData>>());
        renderer->Initialize(nullptr, EntityHandle());
        renderer->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
        renderer->GetMeshGroup()->back().Initialize(
            _defaultCount * _vertexSize,
            _defaultCount * _indexSize);
        return renderer;
    };

    sphereRenderer_  = makeRenderer(kSphereVertexSize,  kSphereIndexSize,  kDefaultMeshCount_);
    boxRenderer_     = makeRenderer(kBoxVertexSize,      kBoxIndexSize,     kDefaultMeshCount_);
    capsuleRenderer_ = makeRenderer(kCapsuleVertexSize,  kCapsuleIndexSize, kDefaultMeshCount_);
    coneRenderer_    = makeRenderer(kConeVertexSize,     kConeIndexSize,    kDefaultMeshCount_);

    sphereMeshItr_  = sphereRenderer_->GetMeshGroup()->begin();
    boxMeshItr_     = boxRenderer_->GetMeshGroup()->begin();
    capsuleMeshItr_ = capsuleRenderer_->GetMeshGroup()->begin();
    coneMeshItr_    = coneRenderer_->GetMeshGroup()->begin();
}

void EmitterShapeRenderingSystem::Update() {
    if (ShouldSkipRender()) {
        return;
    }
    CreateRenderMesh();
    Rendering();
}

void EmitterShapeRenderingSystem::Finalize() {
    sphereRenderer_->Finalize();
    boxRenderer_->Finalize();
    capsuleRenderer_->Finalize();
    coneRenderer_->Finalize();

    dxCommand_->Finalize();
}

void EmitterShapeRenderingSystem::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice*      dxDevice      = Engine::GetInstance()->GetDxDevice();

    shaderManager->LoadShader("ColoredVertex.VS");
    shaderManager->LoadShader("ColoredVertex.PS", kShaderDirectory, L"ps_6_0");

    ShaderInfo shaderInfo{};
    shaderInfo.vsKey = "ColoredVertex.VS";
    shaderInfo.psKey = "ColoredVertex.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[2]{};
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[1].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[1]);
#pragma endregion

#pragma region "InputElement"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc{};
    inputElementDesc.SemanticName      = "POSITION";
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "COLOR";
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    shaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    shaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);

    pso_ = shaderManager->CreatePso(
        "LineMesh_" + kBlendModeStr[int32_t(BlendMode::Alpha)],
        shaderInfo,
        dxDevice->device_);
}

void EmitterShapeRenderingSystem::StartRender() {
    auto& commandList = dxCommand_->GetCommandList();
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    CameraManager::GetInstance()->SetBufferForRootParameter(GetScene(), commandList, 1);
}

/// <summary>
/// 各メッシュをクリアし、ParticleSystem / EntitySpawner の Emitter から描画データを収集する
/// </summary>
void EmitterShapeRenderingSystem::CreateRenderMesh() {
    // ── 全レンダラーをクリア ──────────────────────────────────
    auto clearRenderer = [](LineRenderer* _renderer, std::vector<LineRenderer::MeshType>::iterator& _itr) {
        for (auto& mesh : *_renderer->GetMeshGroup()) {
            mesh.vertexes_.clear();
            mesh.indexes_.clear();
        }
        _itr = _renderer->GetMeshGroup()->begin();
    };

    clearRenderer(sphereRenderer_.get(),  sphereMeshItr_);
    clearRenderer(boxRenderer_.get(),     boxMeshItr_);
    clearRenderer(capsuleRenderer_.get(), capsuleMeshItr_);
    clearRenderer(coneRenderer_.get(),    coneMeshItr_);

    // ── Emitter ────────────────────────────────────────────────
    if (emitters_ && !emitters_->IsEmpty()) {
        for (auto& slot : emitters_->GetSlots()) {
            for (auto& emitter : slot.components) {
                AddShapeToMesh(emitter.emitter_);
            }
        }
    }

    // ── EntitySpawner ──────────────────────────────────────────
    if (spawners_ && !spawners_->IsEmpty()) {
        for (auto& slot : spawners_->GetSlots()) {
            for (auto& spawner : slot.components) {
                AddShapeToMesh(spawner.emitter_);
            }
        }
    }

    sphereMeshItr_->TransferData();
    boxMeshItr_->TransferData();
    capsuleMeshItr_->TransferData();
    coneMeshItr_->TransferData();
}

/// <summary>
/// Emitter のシェイプ種別に応じてメッシュを追加する。
/// worldOriginPos_ を描画の基点として使用する。
/// </summary>
void EmitterShapeRenderingSystem::AddShapeToMesh(const Emitter& _ctrl) {
    if (!_ctrl.spawnShape_) {
        return;
    }

    const Vec3f&   origin = _ctrl.worldOriginPos_;
    constexpr Vec4f kColor = {0.2f, 1.0f, 0.4f, 1.0f}; // 緑

    // メッシュ容量が足りなくなったら新しいメッシュを追加するラムダ
    auto ensureCapacity = [](
        LineRenderer*                                   _renderer,
        std::vector<LineRenderer::MeshType>::iterator&  _itr,
        uint32_t                                        _vertexSize,
        uint32_t                                        _indexSize) {
        if (_itr->GetIndexCapacity() <= 0) {
            _itr->TransferData();
            ++_itr;
            if (_itr == _renderer->GetMeshGroup()->end()) {
                _renderer->GetMeshGroup()->push_back(Mesh<ColorVertexData>());
                _renderer->GetMeshGroup()->back().Initialize(
                    EmitterShapeRenderingSystem::kDefaultMeshCount_ * _vertexSize,
                    EmitterShapeRenderingSystem::kDefaultMeshCount_ * _indexSize);
                _itr = --_renderer->GetMeshGroup()->end();
            }
        }
    };

    switch (_ctrl.shapeType_) {
    case EmitterShapeType::SPHERE: {
        auto* sphere = static_cast<const EmitterSphere*>(_ctrl.spawnShape_.get());
        ensureCapacity(sphereRenderer_.get(), sphereMeshItr_, kSphereVertexSize, kSphereIndexSize);
        BuildSphereMesh(&(*sphereMeshItr_), origin, sphere->radius_, kColor);
        break;
    }
    case EmitterShapeType::BOX: {
        auto* box = static_cast<const EmitterBox*>(_ctrl.spawnShape_.get());
        ensureCapacity(boxRenderer_.get(), boxMeshItr_, kBoxVertexSize, kBoxIndexSize);
        BuildBoxMesh(&(*boxMeshItr_), origin, box->min_, box->max_, box->rotate_, kColor);
        break;
    }
    case EmitterShapeType::CAPSULE: {
        auto* capsule = static_cast<const EmitterCapsule*>(_ctrl.spawnShape_.get());
        ensureCapacity(capsuleRenderer_.get(), capsuleMeshItr_, kCapsuleVertexSize, kCapsuleIndexSize);
        BuildCapsuleMesh(&(*capsuleMeshItr_), origin, capsule->direction_, capsule->radius_, capsule->length_, kColor);
        break;
    }
    case EmitterShapeType::CONE: {
        auto* cone = static_cast<const EmitterCone*>(_ctrl.spawnShape_.get());
        ensureCapacity(coneRenderer_.get(), coneMeshItr_, kConeVertexSize, kConeIndexSize);
        BuildConeMesh(&(*coneMeshItr_), origin, cone->direction_, cone->angle_, cone->length_, kColor);
        break;
    }
    default:
        break;
    }
}

void EmitterShapeRenderingSystem::RenderCall() {
    auto& commandList = dxCommand_->GetCommandList();

    auto renderGroup = [&](LineRenderer* _renderer) {
        _renderer->GetTransformBuff().SetForRootParameter(commandList, 0);
        for (auto& mesh : *_renderer->GetMeshGroup()) {
            if (mesh.indexes_.empty()) {
                continue;
            }
            commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
            commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
            commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indexes_.size()), 1, 0, 0, 0);
        }
    };

    renderGroup(sphereRenderer_.get());
    renderGroup(boxRenderer_.get());
    renderGroup(capsuleRenderer_.get());
    renderGroup(coneRenderer_.get());
}

void EmitterShapeRenderingSystem::Rendering() {
    StartRender();
    RenderCall();
}

bool EmitterShapeRenderingSystem::ShouldSkipRender() const {
    bool noEmitters = !emitters_ || emitters_->IsEmpty();
    bool noSpawners = !spawners_ || spawners_->IsEmpty();
    return noEmitters && noSpawners;
}
