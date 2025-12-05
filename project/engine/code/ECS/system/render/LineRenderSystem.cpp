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

LineRenderSystem::LineRenderSystem() : BaseRenderSystem() {}
LineRenderSystem::~LineRenderSystem() {}

void LineRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();

    for (size_t i = 0; i < kBlendNum; ++i) {
        activeLineRenderersByBlendMode_[i].reserve(100);
    }
}

void LineRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void LineRenderSystem::StartRender() {
    auto& commandList = dxCommand_->GetCommandList();

    int32_t blendIndex = static_cast<int32_t>(currentBlendMode_);

    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CameraManager::GetInstance()->SetBufferForRootParameter(commandList, 1);
}

void LineRenderSystem::DispatchRenderer(Entity* _entity) {
    std::vector<LineRenderer>* renderers = GetComponents<LineRenderer>(_entity);
    if (!renderers) {
        return;
    }

    for (auto& renderer : *renderers) {
        // 描画フラグが立っていないならスキップ
        if (!renderer.IsRender()) {
            continue;
        }
        ///==============================
        /// Transformの更新
        ///==============================
        Transform* entityTransform_ = GetComponent<Transform>(_entity);
        for (int32_t i = 0; i < renderer.GetMeshGroupSize(); ++i) {
            auto& transform = renderer.GetTransformBuff();
            if (transform->parent == nullptr) {
                transform->parent = entityTransform_;
            }
            transform->UpdateMatrix();
            transform.ConvertToBuffer();
        }
        // BlendMode を 適応
        BlendMode rendererBlend = renderer.GetCurrentBlend();

        activeLineRenderersByBlendMode_[static_cast<size_t>(rendererBlend)].push_back(&renderer);
    }
}

bool LineRenderSystem::ShouldSkipRender() const {
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (!activeLineRenderersByBlendMode_[i].empty()) {
            return false;
        }
    }
    return true;
}

void LineRenderSystem::RenderingBy(BlendMode _blend, bool /*_isCulling*/) {
    int32_t blendIndex = static_cast<int32_t>(_blend);
    bool isSkip        = activeLineRenderersByBlendMode_[blendIndex].empty();
    if (isSkip) {
        return;
    }
    ///==============================
    // BlendMode を 適応
    ///==============================
    auto& commandList = dxCommand_->GetCommandListRef();
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());

    for (auto& renderer : activeLineRenderersByBlendMode_[blendIndex]) {
        for (auto& mesh : *renderer->GetMeshGroup()) {
            if (mesh.GetIndexSize() <= 0) {
                continue;
            }
            // 描画
            commandList->IASetVertexBuffers(0, 1, &mesh.GetVertexBufferView());
            commandList->IASetIndexBuffer(&mesh.GetIndexBufferView());
            commandList->DrawIndexedInstanced(mesh.GetIndexSize(), 1, 0, 0, 0);
        }
    }

    // 描画後はクリア
    activeLineRenderersByBlendMode_[blendIndex].clear();
}

void LineRenderSystem::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = OriGine::Engine::GetInstance()->GetDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("LineMesh_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (psoByBlendMode_[i]) {
                continue;
            }
            psoByBlendMode_[i] = shaderManager->GetPipelineStateObj("LineMesh_" + blendModeStr[i]);
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
        if (psoByBlendMode_[i]) {
            continue;
        }
        BlendMode blend           = static_cast<BlendMode>(i);
        lineShaderInfo.blendMode_ = blend;
        psoByBlendMode_[i]        = shaderManager->CreatePso("LineMesh_" + blendModeStr[i], lineShaderInfo, dxDevice->device_);
    }
}
