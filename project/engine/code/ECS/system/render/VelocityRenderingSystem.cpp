#include "VelocityRenderingSystem.h"

///engine
#include "camera/CameraManager.h"
#include "Engine.h"
//directX12
#include "directX12/DxDevice.h"

///math
#include "math/mathEnv.h"

const int32_t VelocityRenderingSystem::defaultMeshCount_ = 1000;

void VelocityRenderingSystem::Initialize() {
    constexpr int32_t kMeshVertexSize = 8;
    constexpr int32_t kMeshIndexSize  = 8;

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    rigidbodies_ = getComponentArray<Rigidbody>();

    velocityRenderer_ = LineRenderer(std::vector<Mesh<ColorVertexData>>());
    velocityRenderer_.Initialize(nullptr);
    velocityRenderer_.getMeshGroup()->push_back(Mesh<ColorVertexData>());
    velocityRenderer_.getMeshGroup()->back().Initialize(
        VelocityRenderingSystem::defaultMeshCount_ * kMeshVertexSize, // 頂点数 (線 + 矢印分)
        VelocityRenderingSystem::defaultMeshCount_ * kMeshIndexSize // インデックス数
    );
    velocityMeshItr_ = velocityRenderer_.getMeshGroup()->begin();

    CreatePso();
}

void VelocityRenderingSystem::Update() {
    CreateRenderMesh();

    if (velocityRenderer_.getMeshGroup()->front().indexes_.empty()) {
        return;
    }

    StartRender();
    RenderCall();
}

void VelocityRenderingSystem::Finalize() {
    dxCommand_->Finalize();
}

void VelocityRenderingSystem::CreateRenderMesh() {
    constexpr float kSideAngleRate = 0.2f;
    constexpr float kArrowLengthRate = 0.3f;

    auto& meshGroup = velocityRenderer_.getMeshGroup();
    for (auto& mesh : *meshGroup) {
        mesh.vertexes_.clear();
        mesh.indexes_.clear();
    }
    velocityMeshItr_ = meshGroup->begin();

    if (!rigidbodies_) {
        return;
    }

    for (auto& [entityIdx, rbIdx] : rigidbodies_->getEntityIndexBind()) {
        Entity* entity = getEntity(entityIdx);
        if (!entity) {
            continue;
        }

        Transform* transform = getComponent<Transform>(entity);
        Rigidbody* rigidbody = getComponent<Rigidbody>(entity);
        if (!transform || !rigidbody) {
            continue;
        }

        Vec3f pos = transform->worldMat[3];
        Vec3f vel = rigidbody->getVelocity();

        if (vel.lengthSq() < kEpsilon) {
            continue; // ゼロベクトルならスキップ
        }

        Vec3f end = pos + vel * 0.5f; // スケール調整

        uint32_t startIndex = (uint32_t)velocityMeshItr_->vertexes_.size();

        // 本体線
        velocityMeshItr_->vertexes_.push_back({Vec4f(pos, 1.f), Vec4f(1, 1, 1, 1)});
        velocityMeshItr_->vertexes_.push_back({Vec4f(end, 1.f), Vec4f(0, 0, 1, 1)});
        velocityMeshItr_->indexes_.push_back(startIndex);
        velocityMeshItr_->indexes_.push_back(startIndex + 1);

        // 矢印
        Vec3f dir  = vel.normalize();
        Vec3f side = Vec3f::Cross(dir, axisY);
        if (side.lengthSq() < kEpsilon) {
            side = Vec3f::Cross(dir, axisX);
        }
        side = side.normalize() * kSideAngleRate;

        Vec3f arrowL = end - dir * kArrowLengthRate + side;
        Vec3f arrowR = end - dir * kArrowLengthRate - side;

        uint32_t idxBase = (uint32_t)velocityMeshItr_->vertexes_.size();
        velocityMeshItr_->vertexes_.push_back({Vec4f(end, 1.f), Vec4f(0, 0, 1, 1)});
        velocityMeshItr_->vertexes_.push_back({Vec4f(arrowL, 1.f), Vec4f(0, 0, 1, 1)});
        velocityMeshItr_->vertexes_.push_back({Vec4f(end, 1.f), Vec4f(0, 0, 1, 1)});
        velocityMeshItr_->vertexes_.push_back({Vec4f(arrowR, 1.f), Vec4f(0, 0, 1, 1)});

        velocityMeshItr_->indexes_.push_back(idxBase);
        velocityMeshItr_->indexes_.push_back(idxBase + 1);
        velocityMeshItr_->indexes_.push_back(idxBase + 2);
        velocityMeshItr_->indexes_.push_back(idxBase + 3);
    }

    velocityMeshItr_->TransferData();
}

void VelocityRenderingSystem::RenderCall() {
    auto commandList = dxCommand_->getCommandList();
    velocityRenderer_.getTransformBuff().SetForRootParameter(commandList, 0);
    for (auto& mesh : *velocityRenderer_.getMeshGroup()) {
        if (mesh.indexes_.empty())
            continue;
        commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
        commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
        commandList->DrawIndexedInstanced((UINT)mesh.indexes_.size(), 1, 0, 0, 0);
    }
}

void VelocityRenderingSystem::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    shaderManager->LoadShader("ColoredVertex.VS");
    shaderManager->LoadShader("ColoredVertex.PS", shaderDirectory, L"ps_6_0");

    ShaderInfo shaderInfo{};
    shaderInfo.vsKey = "ColoredVertex.VS";
    shaderInfo.psKey = "ColoredVertex.PS";

    D3D12_ROOT_PARAMETER rootParameter[2]{};
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[1].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[1]);

    D3D12_INPUT_ELEMENT_DESC inputElementDesc{};
    inputElementDesc.SemanticName      = "POSITION";
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "COLOR";
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);

    shaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    shaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);

    pso_ = shaderManager->CreatePso("VelocityLine", shaderInfo, dxDevice->device_);
}

void VelocityRenderingSystem::StartRender() {
    auto commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}
