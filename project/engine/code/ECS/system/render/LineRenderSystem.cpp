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

void LineRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();

    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        activeLineRenderersByBlendMode_[blend].reserve(100);
    }
}

void LineRenderSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    activeLineRenderersByBlendMode_.clear();

    for (auto& id : entityIDs_) {
        Entity* entity = getEntity(id);
        DispatchRenderer(entity);
    }

    // アクティブなレンダラーが一つもなければ終了
    bool isSkip = true;
    for (const auto& [_, renderers] : activeLineRenderersByBlendMode_) {
        if (!renderers.empty()) {
            isSkip = false;
            break;
        }
    }


    if (isSkip) {
        return;
    }

    StartRender();

    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        RenderingBy(blend);
    }

}

void LineRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void LineRenderSystem::DispatchRenderer(Entity* _entity) {
    std::vector<LineRenderer>* renderers = getComponents<LineRenderer>(_entity);
    if (!renderers) {
        return;
    }

    for (auto& renderer : *renderers) {
        // 描画フラグが立っていないならスキップ
        if (!renderer.isRender()) {
            continue;
        }
        ///==============================
        /// Transformの更新
        ///==============================
        Transform* entityTransform_ = getComponent<Transform>(_entity);
        for (int32_t i = 0; i < renderer.getMeshGroupSize(); ++i) {
            auto& transform = renderer.getTransformBuff();
            if (transform->parent == nullptr) {
                transform->parent = entityTransform_;
            }
            transform->UpdateMatrix();
            transform.ConvertToBuffer();
        }
        // BlendMode を 適応
        BlendMode rendererBlend = renderer.getCurrentBlend();
        this->activeLineRenderersByBlendMode_[rendererBlend].push_back(renderer);
    }
}
void LineRenderSystem::RenderingBy(BlendMode _blend) {
    bool isSkip = activeLineRenderersByBlendMode_[_blend].empty();
    if (isSkip) {
        return;
    }
    ///==============================
    // BlendMode を 適応
    ///==============================
    auto& commandList = dxCommand_->getCommandListRef();
    commandList->SetGraphicsRootSignature(pso_[_blend]->rootSignature.Get());
    commandList->SetPipelineState(pso_[_blend]->pipelineState.Get());

    for (auto& renderer : activeLineRenderersByBlendMode_[_blend]) {
        for (auto& mesh : *renderer.getMeshGroup()) {
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

void LineRenderSystem::UpdateEntity(Entity* _entity) {
    auto commandList       = dxCommand_->getCommandList();
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
            transform->UpdateMatrix();
            transform.ConvertToBuffer();
            transform.SetForRootParameter(commandList, 0);
        }

        ///==============================
        /// 描画
        ///==============================
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
    if (shaderManager->IsRegisteredPipelineStateObj("LineMesh_" + blendModeStr[0])) {
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
        pso_[lineShaderInfo.blendMode_] = shaderManager->CreatePso("LineMesh_" + blendModeStr[i], lineShaderInfo, dxDevice->device_);
    }
}

void LineRenderSystem::StartRender() {
    lineIsStrip_ = false;

    auto commandList = dxCommand_->getCommandList();
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}

void LineRenderSystem::settingPSO(BlendMode _blend) {
    auto commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_[_blend]->rootSignature.Get());
    commandList->SetPipelineState(pso_[_blend]->pipelineState.Get());
}
