#include "ModelMeshRenderer.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"

#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// ecs
#include "component/ComponentArray.h"

// directX12
#include "directX12/DxDevice.h"

// manager
#include "model/ModelManager.h"

#ifdef _DEBUG
/// editor
#include "editor/EditorController.h"

/// util
#include "util/myFileSystem/myFileSystem.h"
#include "util/myGui/MyGui.h"

/// externals
#include <imgui/imgui.h>
#endif // _DEBUG

/// externals
#include <nlohmann/json.hpp>

using namespace OriGine;

//==============================================================================
// ModelMeshRenderer - Core Implementation
//==============================================================================

ModelMeshRenderer::ModelMeshRenderer(const std::vector<TextureColorMesh>& _meshGroup)
    : MeshRenderer<TextureColorMesh, TextureColorVertexData>(_meshGroup) {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    if (meshTextureNumbers_.size() != meshGroup_->size()) {
        meshTextureNumbers_.resize(meshGroup_->size());
        textureFilePath_.resize(meshGroup_->size(), "");
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i]->UpdateMatrix();
        meshTransformBuff_[i].ConvertToBuffer();

        meshMaterialBuff_[i].second = SimpleConstantBuffer<Material>();
        meshMaterialBuff_[i].second.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        meshMaterialBuff_[i].second.ConvertToBuffer(Material());

        meshTextureNumbers_[i] = 0;
        textureFilePath_[i]    = "";
    }
}

ModelMeshRenderer::ModelMeshRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup)
    : MeshRenderer<TextureColorMesh, TextureColorVertexData>(_meshGroup) {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    if (meshTextureNumbers_.size() != meshGroup_->size()) {
        meshTextureNumbers_.resize(meshGroup_->size());
        textureFilePath_.resize(meshGroup_->size(), "");
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

        meshTransformBuff_[i]->UpdateMatrix();
        meshTransformBuff_[i].ConvertToBuffer();

        meshTextureNumbers_[i] = 0;
        textureFilePath_[i]    = "";
    }
}

void ModelMeshRenderer::Initialize(Scene* _scene, EntityHandle _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

    if (!fileName_.empty()) {
        CreateModelMeshRenderer(this, _hostEntity, directory_, fileName_, false);
    }

    InitializeTransformBuffer();
    InitializeMaterialBuffer();

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].openData_.UpdateMatrix();
        meshTransformBuff_[i].ConvertToBuffer();
    }

    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        if (textureFilePath_[i].empty()) {
            continue;
        }
        meshTextureNumbers_[i] = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(textureFilePath_[i]);
    }
}

void ModelMeshRenderer::InitializeTransformBuffer() {
    meshTransformBuff_.resize(meshGroup_->size());
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        meshTransformBuff_[i].ConvertToBuffer();
    }
}

void ModelMeshRenderer::InitializeMaterialBufferWithMaterialIndex() {
    meshMaterialBuff_.resize(meshGroup_->size());

    textureFilePath_.resize(meshGroup_->size(), "");
    meshTextureNumbers_.resize(meshGroup_->size(), 0);

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshMaterialBuff_[i].first = ComponentHandle();
        meshMaterialBuff_[i].second.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}

void ModelMeshRenderer::InitializeMaterialBuffer() {
    meshMaterialBuff_.resize(meshGroup_->size());

    textureFilePath_.resize(meshGroup_->size(), "");
    meshTextureNumbers_.resize(meshGroup_->size(), 0);

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshMaterialBuff_[i].second.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}

//==============================================================================
// ModelMeshRenderer - Helper Functions
//==============================================================================

void OriGine::CreateModelMeshRenderer(
    ModelMeshRenderer* _renderer,
    EntityHandle _hostEntity,
    const std::string& _directory,
    const std::string& _fileName,
    bool _usingDefaultTexture) {
    bool isLoaded = false;

    if (!_renderer->GetMeshGroup()->empty()) {
        _renderer->GetMeshGroup()->clear();
    }

    // -------------------- Modelの読み込み --------------------//
    auto model = ModelManager::GetInstance()->Create(_directory, _fileName, [&_hostEntity, &_renderer, &isLoaded, _usingDefaultTexture](Model* _model) {
        // 再帰ラムダをstd::functionとして定義
        std::function<void(ModelMeshRenderer*, Model*, ModelNode*)> CreateMeshGroupFormNode;
        CreateMeshGroupFormNode = [&](ModelMeshRenderer* _meshRenderer, Model* _innerModel, ModelNode* _node) {
            auto meshItr = _innerModel->meshData_->meshGroup.find(_node->name);
            if (meshItr != _innerModel->meshData_->meshGroup.end()) {
                _meshRenderer->PushBackMesh(meshItr->second);
            }
            for (auto& child : _node->children) {
                CreateMeshGroupFormNode(_meshRenderer, _innerModel, &child);
            }
        };

        // メッシュグループの作成
        CreateMeshGroupFormNode(_renderer, _model, &_model->meshData_->rootNode);

        isLoaded = true;
    });

    // ロードが完了するまで待機
    while (!isLoaded) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void OriGine::InitializeMaterialFromModelFile(
    ModelMeshRenderer* _renderer,
    Scene* _scene,
    EntityHandle _hostEntity,
    const std::string& _directory,
    const std::string& _fileName) {
    if (!_renderer->GetMeshGroup()->empty()) {
        _renderer->GetMeshGroup()->clear();
    }

    // -------------------- Modelの取得 --------------------//
    const auto& defaultModelMaterial = ModelManager::GetInstance()->GetDefaultMaterials(_directory, _fileName);

    // -------------------- Materialの初期化 --------------------//
    auto materialArray = _scene->GetComponentArray<Material>();
    auto& materials    = materialArray->GetComponents(_hostEntity);
    // Bufferの初期化
    _renderer->InitializeMaterialBuffer();
    // nullptr の場合は,生成から
    if (materials.empty()) {
        for (int32_t i = 0; i < static_cast<int32_t>(_renderer->GetMeshGroupSize()); ++i) {
            // 生成&初期化
            ComponentHandle createdMaterialHandle = materialArray->AddComponent(_scene, _hostEntity);
            Material* material                    = materialArray->GetComponent(createdMaterialHandle);
            *material                             = defaultModelMaterial[i].material.openData_;

            _renderer->SetMaterialHandle(i, createdMaterialHandle);
        }
    } else {
        for (int32_t i = 0; i < static_cast<int32_t>(_renderer->GetMeshGroupSize()); ++i) {
            // マテリアルの設定
            ComponentHandle materialHandle = _renderer->GetMaterialHandle(i);
            if (!materialHandle.IsValid()) {
                // 生成&初期化
                ComponentHandle createdMaterialHandle = materialArray->AddComponent(_scene, _hostEntity);
                Material* material                    = materialArray->GetComponent(createdMaterialHandle);
                *material                             = defaultModelMaterial[i].material.openData_;

                _renderer->SetMaterialHandle(i, createdMaterialHandle);
                continue;
            }
            Material* material = materialArray->GetComponent(materialHandle);
            *material          = defaultModelMaterial[i].material.openData_;
        }
    }
    for (uint32_t i = 0; i < static_cast<uint32_t>(_renderer->GetMeshGroupSize()); ++i) {
        _renderer->SetTexture(i, defaultModelMaterial[i].texturePath);
    }
}

//==============================================================================
// ModelMeshRenderer - Editor (Debug Only)
//==============================================================================

void ModelMeshRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    std::string label = "isRender##" + _parentLabel;
    CheckBoxCommand(label, isRender_);

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), kBlendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : kBlendModeStr) {
            isSelected = blendModeName == kBlendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));

                break;
            }

            blendIndex++;
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Model File: %s", fileName_.c_str());

    label = "LoadModel##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"obj", "gltf", "dds"})) {
            auto SetDirectory                          = std::make_unique<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory);
            auto SetName                               = std::make_unique<SetterCommand<std::string>>(&fileName_, fileName);
            std::unique_ptr<CommandCombo> commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::move(SetDirectory));
            commandCombo->AddCommand(std::move(SetName));
            commandCombo->SetFuncOnAfterCommand([this, _scene]() {
                if (!this) {
                    return;
                }
                this->Finalize();
                meshGroup_ = std::make_shared<std::vector<TextureColorMesh>>();
                CreateModelMeshRenderer(this, this->hostEntityHandle_, this->directory_, this->fileName_);
                InitializeMaterialFromModelFile(this, _scene, this->hostEntityHandle_, this->directory_, this->fileName_);
            },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ImGui::Separator();

    std::string meshName = "Mesh##" + _parentLabel;

    auto& materials            = _scene->GetComponents<Material>(_handle);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size());
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshName = std::format("Mesh [{}]##", i) + _parentLabel;
        if (ImGui::CollapsingHeader(meshName.c_str())) {
            ImGui::Indent();

            ImGui::Text("Texture Directory: %s", textureFilePath_[i].c_str());
            if (AskLoadTextureButton(meshTextureNumbers_[i], meshName)) {
                std::string outputPath = "";
                if (OpenFileDialog(kApplicationResourceDirectory, outputPath, {"png"})) {
                    auto SetPathCommand = std::make_unique<SetterCommand<std::string>>(&textureFilePath_[i], outputPath,
                        [this, i](std::string* _path) {
                            meshTextureNumbers_[i] = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(*_path);
                        });

                    OriGine::EditorController::GetInstance()->PushCommand(std::move(SetPathCommand));
                }
            }

            label = "Transform##" + _parentLabel;
            if (ImGui::TreeNode(label.c_str())) {
                Transform& transform = meshTransformBuff_[i].openData_;
                // Transform
                transform.Edit(_scene, _handle, meshName);
                meshTransformBuff_[i].ConvertToBuffer();

                ImGui::TreePop();
            }

            label = "MaterialIndex##" + _parentLabel;

            ComponentHandle& materialHandle = meshMaterialBuff_[i].first;
            int32_t materialIndex           = -1;
            if (materialHandle.IsValid()) {
                for (int32_t mIndex = 0; mIndex < entityMaterialSize; ++mIndex) {
                    if (materials[mIndex].GetHandle() == materialHandle) {
                        materialIndex = mIndex;
                        break;
                    }
                }
            }

            ImGui::InputInt(label.c_str(), &materialIndex);
            materialIndex = std::clamp(materialIndex, -1, entityMaterialSize - 1);
            if (materialIndex >= 0) {
                // 選択されたものと違う場合は変更
                if (materialHandle != materials[materialIndex].GetHandle()) {
                    auto command = std::make_unique<SetterCommand<ComponentHandle>>(
                        &meshMaterialBuff_[i].first, materials[materialIndex].GetHandle());
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                }

                label = "Material##" + _parentLabel;
                if (ImGui::TreeNode(label.c_str())) {
                    // Material編集
                    materials[materialIndex].Edit(_scene, _handle, meshName);
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }
    }
#endif // _DEBUG
}

//==============================================================================
// ModelMeshRenderer - JSON Serialization
//==============================================================================

namespace OriGine {

void to_json(nlohmann::json& _j, const ModelMeshRenderer& _comp) {
    _j["directory"] = _comp.directory_;
    _j["fileName"]  = _comp.fileName_;

    _j["isRender"]  = _comp.isRender_;
    _j["blendMode"] = static_cast<int32_t>(_comp.currentBlend_);

    // Transform
    nlohmann::json transformBufferData = nlohmann::json::array();
    for (uint32_t i = 0; i < _comp.meshTransformBuff_.size(); ++i) {
        nlohmann::json transformData;
        to_json(transformData, _comp.meshTransformBuff_[i].openData_);
        transformBufferData.emplace_back(transformData);
    }
    _j["transformBufferDatas"] = transformBufferData;

    // Material Index(ComponentHandle)
    nlohmann::json materialBufferDatas = nlohmann::json::array();
    for (int32_t i = 0; i < _comp.meshGroup_->size(); ++i) {
        nlohmann::json bufferData;
        bufferData["Handle"] = _comp.meshMaterialBuff_[i].first;
        materialBufferDatas.push_back(bufferData);
    }
    _j["materialIndexDatas"] = materialBufferDatas;

    // Texture(ファイルパス)
    nlohmann::json textureFilePath = nlohmann::json::array();
    for (uint32_t i = 0; i < _comp.textureFilePath_.size(); ++i) {
        textureFilePath.emplace_back(_comp.textureFilePath_[i]);
    }
    _j["textureFilePath"] = textureFilePath;
}

void from_json(const nlohmann::json& _j, ModelMeshRenderer& _comp) {
    _j.at("directory").get_to(_comp.directory_);
    _j.at("fileName").get_to(_comp.fileName_);

    _j.at("isRender").get_to(_comp.isRender_);
    int32_t blendMode = 0;
    _j.at("blendMode").get_to(blendMode);
    _comp.currentBlend_ = static_cast<BlendMode>(blendMode);

    // Transform
    _comp.meshTransformBuff_.clear();
    auto& transformBufferData = _j.at("transformBufferDatas");
    for (auto& transformData : transformBufferData) {
        IConstantBuffer<Transform> transformBuff;
        transformBuff.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        from_json(transformData, transformBuff.openData_);
        transformBuff.openData_.UpdateMatrix();
        transformBuff.ConvertToBuffer();
        _comp.meshTransformBuff_.emplace_back(std::move(transformBuff));
    }

    // Material Index
    _comp.meshMaterialBuff_.clear();
    if (_j.find("materialIndexDatas") != _j.end()) {
        auto& materialBufferDatas = _j.at("materialIndexDatas");
        for (auto& materialData : materialBufferDatas) {
            auto& backMaterial = _comp.meshMaterialBuff_.emplace_back(std::make_pair(ComponentHandle(), SimpleConstantBuffer<Material>()));
            if (materialData.contains("Handle")) {
                materialData["Handle"].get_to<ComponentHandle>(backMaterial.first);
            }
        }
    }

    // Texture
    _comp.textureFilePath_.clear();
    _comp.meshTextureNumbers_.clear();
    auto& textureFilePath = _j.at("textureFilePath");
    for (auto& texturePath : textureFilePath) {
        _comp.textureFilePath_.emplace_back(texturePath);
        _comp.meshTextureNumbers_.emplace_back(0);
    }
}

} // namespace OriGine
