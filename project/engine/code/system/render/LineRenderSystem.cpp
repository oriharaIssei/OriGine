#include "LineRenderSystem.h"

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
#include "ECSManager.h"

void LineRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();
}

void LineRenderSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    StartRender();

    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        UpdateEntity(entity);
    }
}

void LineRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void LineRenderSystem::UpdateEntity(GameEntity* _entity) {
    auto commandList      = dxCommand_->getCommandList();
    int32_t componentIndex = 0;
    while (true) {
        LineRenderer* renderer = getComponent<LineRenderer>(_entity, componentIndex++);
        // nullptr なら これ以上存在しないとして終了
        if (!renderer) {
            return;
        }
        // 描画フラグが立っていないならスキップ
        if (!renderer->isRender()) {
            continue;
        }
        ///==============================
        /// Transformの更新
        ///==============================
        {
            Transform* entityTransform_ = getComponent<Transform>(_entity);
            auto& transform             = renderer->getTransformBuff();
            if (transform->parent == nullptr) {
                transform->parent = entityTransform_;
            }
            transform.openData_.Update();
            transform.ConvertToBuffer();
            transform.SetForRootParameter(commandList, 0);
        }

        ///==============================
        /// 描画
        ///==============================

        if (currentBlend_ != renderer->getCurrentBlend()) {
            currentBlend_ = renderer->getCurrentBlend();

            commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
            commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
        }

        if (lineIsStrip_ != renderer->isLineStrip()) {
            lineIsStrip_ = renderer->isLineStrip();
            if (lineIsStrip_) {
                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
            } else {
                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
            }
        }

        for (auto& mesh : *renderer->getMeshGroup()) {
            if (mesh.getIndexSize() <= 0) {
                continue;
            }
            // 描画
            commandList->IASetVertexBuffers(0, 1, &mesh.getVertexBufferView());
            commandList->IASetIndexBuffer(&mesh.getIndexBufferView());
            commandList->DrawIndexedInstanced(mesh.getIndexSize(), 1, 0, 0, 0);
        }
    }
}

void LineRenderSystem::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegistertedPipelineStateObj("LineMesh_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            BlendMode blend = static_cast<BlendMode>(i);
            if (pso_[blend]) {
                continue;
            }
            pso_[blend] = shaderManager->getPipelineStateObj("LineMesh_" + blendModeStr[i]);
        }
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
        pso_[lineShaderInfo.blendMode_] = shaderManager->CreatePso("LineMesh_" + blendModeStr[i], lineShaderInfo, dxDevice->getDevice());
    }
}

void LineRenderSystem::StartRender() {
    currentBlend_ = BlendMode::Alpha;
    lineIsStrip_  = false;

    auto commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}
