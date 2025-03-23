#include "MeshRenderer.h"

/// engine
#include "Engine.h"
// Ecs
#include "ECSManager.h"
// assets
#include "model/Model.h"
// manager
#include "model/ModelManager.h"

#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

#include "imgui/imgui.h"

//----------------------------------------------------------------------------------------------------------
// ↓ DefaultMeshRenderer
//----------------------------------------------------------------------------------------------------------
#pragma region "ModelMeshRenderer"
ModelMeshRenderer::ModelMeshRenderer(const std::vector<TextureMesh>& _meshGroup)
    : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    if (meshTextureNumber_.size() != meshGroup_->size()) {
        meshTextureNumber_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

        meshTransformBuff_[i]->Update();
        meshTransformBuff_[i].ConvertToBuffer();

        meshMaterialBuff_[i]  = Engine::getInstance()->getMaterialManager()->getMaterial("white");
        meshTextureNumber_[i] = 0;
    }
}

ModelMeshRenderer::ModelMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup)
    : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    if (meshTextureNumber_.size() != meshGroup_->size()) {
        meshTextureNumber_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

        meshTransformBuff_[i]->Update();
        meshTransformBuff_[i].ConvertToBuffer();

        meshMaterialBuff_[i]  = Engine::getInstance()->getMaterialManager()->getMaterial("white");
        meshTextureNumber_[i] = 0;
    }
}

void ModelMeshRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    Transform* entityTransform = getComponent<Transform>(_hostEntity);
    meshTransformBuff_.resize(meshGroup_->size());
    meshTextureNumber_.resize(meshGroup_->size());
    meshMaterialBuff_.resize(meshGroup_->size());

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {

        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        if (meshTransformBuff_[i]->parent == nullptr) {
            meshTransformBuff_[i]->parent = entityTransform;
        }

        meshTransformBuff_[i].openData_.Update();
        meshTransformBuff_[i].ConvertToBuffer();
    }
}

bool ModelMeshRenderer::Edit() {
#ifdef _DEBUG
    bool isChange = false;

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##BlendMode", blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                currentBlend_ = static_cast<BlendMode>(blendIndex);

                isChange = true;
                break;
            }

            blendIndex++;
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Model File: %s", fileName_.c_str());

    if (ImGui::Button("Load")) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"obj", "gltf"})) {
            directory_ = kApplicationResourceDirectory + "/" + directory;
            fileName_  = fileName;

            this->Finalize();
            meshGroup_ = std::make_shared<std::vector<TextureMesh>>();
            CreateModelMeshRenderer(this, this->hostEntity_, directory_, fileName_);

            isChange = true;
        }
    }

    ImGui::Separator();

    std::string meshName = "Mesh";
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshName = "Mesh" + '[' + std::to_string(i) + "]";
        if (ImGui::CollapsingHeader(meshName.c_str())) {
            if (ImGui::TreeNode("Transform")) {
                Transform& transform = meshTransformBuff_[i].openData_;
                // Transform
                if (transform.Edit()) {
                    meshTransformBuff_[i].ConvertToBuffer();
                    isChange = true;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Material")) {

                ImGui::TreePop();
            }
        }
    }
    return isChange;
#else
    return false;
#endif // _DEBUG
}

void ModelMeshRenderer::Save(BinaryWriter& _writer) {
    MeshRenderer::Save(_writer);

    _writer.Write<std::string>(directory_);
    _writer.Write<std::string>(fileName_);
}

void ModelMeshRenderer::Load(BinaryReader& _reader) {
    MeshRenderer::Load(_reader);

    _reader.Read<std::string>(directory_);
    _reader.Read<std::string>(fileName_);

    if (!fileName_.empty()) {
        CreateModelMeshRenderer(this, hostEntity_, directory_, fileName_);
    }
}

#pragma endregion

#pragma region "PrimitiveMeshRenderer"
PrimitiveMeshRenderer::PrimitiveMeshRenderer()
    : MeshRenderer<PrimitiveMesh, PrimitiveVertexData>() {}

PrimitiveMeshRenderer::PrimitiveMeshRenderer(const std::vector<PrimitiveMesh>& _meshGroup)
    : MeshRenderer<PrimitiveMesh, PrimitiveVertexData>(_meshGroup) {

    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        meshMaterialBuff_[i] = Engine::getInstance()->getMaterialManager()->getMaterial("white");
    }
}

PrimitiveMeshRenderer::PrimitiveMeshRenderer(const std::shared_ptr<std::vector<PrimitiveMesh>>& _meshGroup)
    : MeshRenderer<PrimitiveMesh, PrimitiveVertexData>(_meshGroup) {

    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        meshMaterialBuff_[i] = Engine::getInstance()->getMaterialManager()->getMaterial("white");
    }
}

PrimitiveMeshRenderer::~PrimitiveMeshRenderer() {}

void PrimitiveMeshRenderer::Initialize(GameEntity* _hostEntity) {
    Transform* entityTransform = getComponent<Transform>(_hostEntity);
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        if (meshTransformBuff_[i]->parent == nullptr) {
            meshTransformBuff_[i]->parent = entityTransform;
        }

        meshTransformBuff_[i].openData_.Update();
        meshTransformBuff_[i].ConvertToBuffer();
    }
}

// void PrimitiveMeshRenderer::Update() {
//     for (auto& transform : meshTransformBuff_) {
//         transform.openData_.Update();
//         transform.ConvertToBuffer();
//     }
// }
//
// void PrimitiveMeshRenderer::Render() {
//     IRendererComponentController* controller = RenderManager::getInstance()->getRendererController<PrimitiveMeshRendererController>();
//     auto* commandList                        = controller->getDxCommand()->getCommandList();
//
//     uint32_t index = 0;
//
//     for (auto& mesh : *meshGroup_) {
//         auto& material = meshMaterialBuff_[index];
//
//         IConstantBuffer<Transform>& meshTransform = meshTransformBuff_[index];
//         meshTransform.ConvertToBuffer();
//
//         commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
//         commandList->IASetIndexBuffer(&mesh.ibView);
//
//         meshTransform.SetForRootParameter(commandList, 0);
//
//         material->SetForRootParameter(commandList, 2);
//         // 描画!!!
//         commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);
//
//         ++index;
//     }
// }
#pragma endregion

//----------------------------------------------------------------------------------------------------------
// ↓ PrimitiveMeshContorller
//----------------------------------------------------------------------------------------------------------
// void PrimitiveMeshRendererController::StartRender() {
//    auto* commandList = dxCommand_->getCommandList();
//    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
//    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
//
//    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//}
//
// void PrimitiveMeshRendererController::CreatePso() {
//    ShaderManager* shaderManager = ShaderManager::getInstance();
//    ///=================================================
//    /// shader読み込み
//    ///=================================================
//
//    shaderManager->LoadShader("Object3d.VS");
//    shaderManager->LoadShader("Object3d.PS", shaderDirectory, L"ps_6_0");
//
//    ///=================================================
//    /// shader情報の設定
//    ///=================================================
//    ShaderInfo primShaderInfo;
//    primShaderInfo.vsKey = "Object3d.VS";
//    primShaderInfo.psKey = "Object3d.PS";
//
// #pragma region "RootParameter"
//    D3D12_ROOT_PARAMETER rootParameter[7]{};
//    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
//    // PixelShaderで使う
//    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
//    // レジスタ番号0 とバインド
//    // register(b0) の 0. b11 なら 11
//    rootParameter[0].Descriptor.ShaderRegister = 0;
//    primShaderInfo.pushBackRootParameter(rootParameter[0]);
//
//    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
//    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
//    rootParameter[1].Descriptor.ShaderRegister = 2;
//    primShaderInfo.pushBackRootParameter(rootParameter[1]);
//
//    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
//    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
//    rootParameter[2].Descriptor.ShaderRegister = 0;
//    primShaderInfo.pushBackRootParameter(rootParameter[2]);
//
//    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
//    rootParameter[3].Descriptor.ShaderRegister = 1; // t1 register for DirectionalLight StructuredBuffer
//    primShaderInfo.pushBackRootParameter(rootParameter[3]);
//
//    // PointLight ... 4 (StructuredBuffer)
//    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
//    rootParameter[4].Descriptor.ShaderRegister = 3; // t3 register for PointLight StructuredBuffer
//    primShaderInfo.pushBackRootParameter(rootParameter[4]);
//
//    // SpotLight ... 5 (StructuredBuffer)
//    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
//    rootParameter[5].Descriptor.ShaderRegister = 4; // t4 register for SpotLight StructuredBuffer
//    primShaderInfo.pushBackRootParameter(rootParameter[5]);
//    // lightCounts
//    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
//    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
//    rootParameter[6].Descriptor.ShaderRegister = 5;
//    primShaderInfo.pushBackRootParameter(rootParameter[6]);
//
//    D3D12_DESCRIPTOR_RANGE directionalLightRange[1]            = {};
//    directionalLightRange[0].BaseShaderRegister                = 1;
//    directionalLightRange[0].NumDescriptors                    = 1;
//    directionalLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//    directionalLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//    D3D12_DESCRIPTOR_RANGE pointLightRange[1]            = {};
//    pointLightRange[0].BaseShaderRegister                = 3;
//    pointLightRange[0].NumDescriptors                    = 1;
//    pointLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//    pointLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//    D3D12_DESCRIPTOR_RANGE spotLightRange[1]            = {};
//    spotLightRange[0].BaseShaderRegister                = 4;
//    spotLightRange[0].NumDescriptors                    = 1;
//    spotLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//    spotLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//    primShaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, 3);
//    primShaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, 4);
//    primShaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, 5);
// #pragma endregion
//
// #pragma region "Input Element"
//    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
//    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
//    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
//    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
//    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
//    primShaderInfo.pushBackInputElementDesc(inputElementDesc);
//
//    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
//    inputElementDesc.SemanticIndex     = 0;
//    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
//    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//    primShaderInfo.pushBackInputElementDesc(inputElementDesc);
// #pragma endregion
//
//    ///=================================================
//    /// BlendMode ごとの Pso作成
//    ///=================================================
//    primShaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
//    for (size_t i = 0; i < kBlendNum; ++i) {
//        primShaderInfo.blendMode_       = static_cast<BlendMode>(i);
//        pso_[primShaderInfo.blendMode_] = shaderManager->CreatePso("PrimitiveMesh_" + blendModeStr[i], primShaderInfo, dxDevice_->getDevice());
//    }
//};

void CreateModelMeshRenderer(ModelMeshRenderer* _renderer, GameEntity* _hostEntity, const std::string& _directory, const std::string& _filenName) {
    _renderer->setParentTransform(getComponent<Transform>(_hostEntity));

    bool isLoaded = false;
    // -------------------- Modelの読み込み --------------------//
    ModelManager::getInstance()->Create(_directory, _filenName, [&_hostEntity, &_renderer, &isLoaded](Model* model) {
        // 再帰ラムダをstd::functionとして定義
        std::function<void(ModelMeshRenderer*, Model*, ModelNode*)> CreateMeshGroupFormNode;
        CreateMeshGroupFormNode = [&](ModelMeshRenderer* _meshRenderer, Model* _model, ModelNode* _node) {
            auto meshItr = _model->meshData_->meshGroup_.find(_node->name);
            if (meshItr != _model->meshData_->meshGroup_.end()) {
                _meshRenderer->pushBackMesh(meshItr->second);
            }
            for (auto& child : _node->children) {
                CreateMeshGroupFormNode(_meshRenderer, _model, &child);
            }
            return;
        };

        CreateMeshGroupFormNode(_renderer, model, &model->meshData_->rootNode);
        _renderer->Initialize(_hostEntity);

        // マテリアルの設定
        for (uint32_t i = 0; i < static_cast<uint32_t>(model->materialData_.size()); ++i) {
            _renderer->setMaterialBuff(i, model->materialData_[i].material);
            _renderer->setTextureNumber(i, model->materialData_[i].textureNumber);
        }
        isLoaded = true;
    });

    while (true) {
        if (isLoaded) {
            break;
        }
    }
}
