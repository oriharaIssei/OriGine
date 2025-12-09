#include "MeshRenderer.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"
/// ecs
#include "component/ComponentArray.h"
// directX12
#include "directX12/DxDevice.h"

// manager
#include "model/ModelManager.h"

#define RESOURCE_DIRECTORY
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "engine/EngineInclude.h"

#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

//----------------------------------------------------------------------------------------------------------
// ↓ DefaultMeshRenderer
//----------------------------------------------------------------------------------------------------------
#pragma region "ModelMeshRenderer"
void to_json(nlohmann::json& j, const ModelMeshRenderer& r) {
    j["isRender"]  = r.isRender_;
    j["blendMode"] = static_cast<int32_t>(r.currentBlend_);

    j["directory"] = r.directory_;
    j["fileName"]  = r.fileName_;

    nlohmann::json transformBufferDatas = nlohmann::json::array();
    for (int32_t i = 0; i < r.meshGroup_->size(); ++i) {
        nlohmann::json bufferData;
        to_json(bufferData, r.meshTransformBuff_[i].openData_);
        transformBufferDatas.push_back(bufferData);
    }
    j["transformBufferDatas"] = transformBufferDatas;

    nlohmann::json materialBufferDatas = nlohmann::json::array();
    for (int32_t i = 0; i < r.meshGroup_->size(); ++i) {
        nlohmann::json bufferData;
        to_json(bufferData, r.meshMaterialBuff_[i].first);
        materialBufferDatas.push_back(bufferData);
    }
    j["materialIndexDatas"] = materialBufferDatas;

    nlohmann::json texturePaths = nlohmann::json::array();
    for (const auto& texturePath : r.textureFilePath_) {
        texturePaths.push_back(texturePath);
    }
    j["textureFilePath"] = texturePaths;
}

void from_json(const nlohmann::json& j, ModelMeshRenderer& r) {
    j.at("isRender").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);

    j.at("directory").get_to(r.directory_);
    j.at("fileName").get_to(r.fileName_);

    auto& transformBufferDatas = j.at("transformBufferDatas");
    for (auto& transformData : transformBufferDatas) {
        auto& backTransform     = r.meshTransformBuff_.emplace_back(IConstantBuffer<Transform>());
        backTransform.openData_ = transformData;
    }

    if (j.find("materialIndexDatas") != j.end()) {
        auto& materialBufferDatas = j.at("materialIndexDatas");
        for (auto& materialData : materialBufferDatas) {
            auto& backMaterial = r.meshMaterialBuff_.emplace_back(std::make_pair(-1, SimpleConstantBuffer<Material>()));
            backMaterial.first = materialData;
        }
    }

    if (j.find("textureFilePath") != j.end()) {
        r.textureFilePath_.clear();
        auto& texturePaths = j.at("textureFilePath");
        for (const auto& texturePath : texturePaths) {
            auto& texture = r.textureFilePath_.emplace_back(texturePath.get<std::string>());
            texture       = texturePath;
        }
    }
}

ModelMeshRenderer::ModelMeshRenderer(const std::vector<TextureMesh>& _meshGroup)
    : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {
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

ModelMeshRenderer::ModelMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup)
    : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {
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

void ModelMeshRenderer::Initialize(Entity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    if (!fileName_.empty()) {
        CreateModelMeshRenderer(this, hostEntity_, directory_, fileName_, false);
    }

    InitializeTransformBuffer(_hostEntity);
    InitializeMaterialBuffer(_hostEntity);

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        /// ---------------------------------------------------
        // Transform parent
        /// ---------------------------------------------------
        meshTransformBuff_[i].openData_.UpdateMatrix();
        meshTransformBuff_[i].ConvertToBuffer();
    }

    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        if (textureFilePath_[i].empty()) {
            continue;
        }
        meshTextureNumbers_[i] = TextureManager::LoadTexture(textureFilePath_[i]);
    }
}

void ModelMeshRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    std::string label = "isRender##" + _parentLabel;
    CheckBoxCommand(label, isRender_);

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::GetInstance()->PushCommand(
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
            auto SetDirectory = std::make_unique<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory);
            auto SetName      = std::make_unique<SetterCommand<std::string>>(&fileName_, fileName);
            CommandCombo commandCombo;
            commandCombo.AddCommand(std::move(SetDirectory));
            commandCombo.AddCommand(std::move(SetName));
            commandCombo.SetFuncOnAfterCommand([this, _scene]() {
                if (!this) {
                    return;
                }
                this->Finalize();
                meshGroup_ = std::make_shared<std::vector<TextureMesh>>();
                CreateModelMeshRenderer(this, this->hostEntity_, this->directory_, this->fileName_);
                InitializeMaterialFromModelFile(this, _scene, this->hostEntity_, this->directory_, this->fileName_);
            },
                true);
            EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    }

    ImGui::Separator();

    std::string meshName       = "Mesh##" + _parentLabel;

    auto materials             = _scene->GetComponents<Material>(_entity);
    int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;
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
                            meshTextureNumbers_[i] = TextureManager::LoadTexture(*_path);
                        });

                    EditorController::GetInstance()->PushCommand(std::move(SetPathCommand));
                }
            }

            label = "Transform##" + _parentLabel;
            if (ImGui::TreeNode(label.c_str())) {
                Transform& transform = meshTransformBuff_[i].openData_;
                // Transform
                transform.Edit(_scene, _entity, meshName);
                meshTransformBuff_[i].ConvertToBuffer();

                ImGui::TreePop();
            }

            label                      = "MaterialIndex##" + _parentLabel;
            int32_t& materialIndex     = meshMaterialBuff_[i].first;

            InputGuiCommand(label, materialIndex);
            materialIndex = std::clamp(materialIndex, -1, entityMaterialSize - 1);
            if (materialIndex >= 0) {
                label = "Material##" + _parentLabel;
                if (ImGui::TreeNode(label.c_str())) {
                    materials->operator[](materialIndex).Edit(_scene, _entity, meshName);
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }
    }

#endif // _DEBUG
}

void ModelMeshRenderer::InitializeTransformBuffer(Entity* _hostEntity) {
    hostEntity_ = _hostEntity;
    meshTransformBuff_.resize(meshGroup_->size());
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        meshTransformBuff_[i].ConvertToBuffer();
    }
}

void ModelMeshRenderer::InitializeMaterialBufferWithMaterialIndex(Entity* _hostEntity) {
    hostEntity_ = _hostEntity;

    meshMaterialBuff_.resize(meshGroup_->size());

    textureFilePath_.resize(meshGroup_->size(), "");
    meshTextureNumbers_.resize(meshGroup_->size(), 0);

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshMaterialBuff_[i].first = -1;
        meshMaterialBuff_[i].second.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}

void ModelMeshRenderer::InitializeMaterialBuffer(Entity* _hostEntity) {
    hostEntity_ = _hostEntity;

    meshMaterialBuff_.resize(meshGroup_->size());

    textureFilePath_.resize(meshGroup_->size(), "");
    meshTextureNumbers_.resize(meshGroup_->size(), 0);

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshMaterialBuff_[i].second.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}

#pragma endregion

void CreateModelMeshRenderer(ModelMeshRenderer* _renderer, Entity* _hostEntity, const std::string& _directory, const std::string& _fileName, bool _usingDefaultTexture) {
    bool isLoaded = false;

    if (!_renderer->GetMeshGroup()->empty()) {
        _renderer->GetMeshGroup()->clear();
    }

    // -------------------- Modelの読み込み --------------------//
    auto model = ModelManager::GetInstance()->Create(_directory, _fileName, [&_hostEntity, &_renderer, &isLoaded, _usingDefaultTexture](Model* model) {
        // 再帰ラムダをstd::functionとして定義
        std::function<void(ModelMeshRenderer*, Model*, ModelNode*)> CreateMeshGroupFormNode;
        CreateMeshGroupFormNode = [&](ModelMeshRenderer* _meshRenderer, Model* _model, ModelNode* _node) {
            auto meshItr = _model->meshData_->meshGroup.find(_node->name);
            if (meshItr != _model->meshData_->meshGroup.end()) {
                _meshRenderer->PushBackMesh(meshItr->second);
            }
            for (auto& child : _node->children) {
                CreateMeshGroupFormNode(_meshRenderer, _model, &child);
            }
        };

        // メッシュグループの作成
        CreateMeshGroupFormNode(_renderer, model, &model->meshData_->rootNode);

        isLoaded = true;
    });

    // ロードが完了するまで待機
    while (!isLoaded) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void InitializeMaterialFromModelFile(ModelMeshRenderer* _renderer, Scene* _scene, Entity* _hostEntity, const std::string& _directory, const std::string& _fileName) {
    if (!_renderer->GetMeshGroup()->empty()) {
        _renderer->GetMeshGroup()->clear();
    }

    // -------------------- Modelの取得 --------------------//
    const auto& defaultModelMaterial = ModelManager::GetInstance()->GetDefaultMaterials(_directory, _fileName);

    // -------------------- Materialの初期化 --------------------//
    auto materialArray = _scene->GetComponentArray<Material>();
    auto materials     = materialArray->GetComponents(_hostEntity);
    // Bufferの初期化
    _renderer->InitializeMaterialBuffer(_hostEntity);
    // nullptr の場合は,生成から
    if (materials == nullptr) {
        for (int32_t i = 0; i < static_cast<int32_t>(_renderer->GetMeshGroupSize()); ++i) {
            // 生成&初期化
            int32_t createdMaterialIndex = materialArray->Add(_hostEntity, defaultModelMaterial[i].material.openData_);
            _renderer->SetMaterialIndex(i, createdMaterialIndex);
        }
    } else {
        for (int32_t i = 0; i < static_cast<int32_t>(_renderer->GetMeshGroupSize()); ++i) {
            // マテリアルの設定
            int32_t materialIndex = _renderer->GetMaterialIndex(i);
            if (materialIndex < 0) {
                int32_t createdMaterialIndex = materialArray->Add(_hostEntity, defaultModelMaterial[i].material.openData_);
                _renderer->SetMaterialIndex(i, createdMaterialIndex);
                continue;
            }
            materials->operator[](materialIndex) = defaultModelMaterial[i].material.openData_;
        }
    }
    for (uint32_t i = 0; i < static_cast<uint32_t>(_renderer->GetMeshGroupSize()); ++i) {
        _renderer->SetTexture(i, defaultModelMaterial[i].texturePath);
    }
}

#pragma region "LineRenderer"
#ifdef _DEBUG

/// <summary>
/// LineRenderにLineを追加するコマンド
/// </summary>
class AddLineCommand
    : public IEditCommand {
public:
    AddLineCommand(std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& meshGroup)
        : meshGroup_(meshGroup), addedMeshIndex_(-1), vertex1Index_(-1), vertex2Index_(-1) {}

    void Execute() override {
        Mesh<ColorVertexData>* mesh = &meshGroup_->back();

        // 新しいメッシュが必要な場合
        if (mesh->GetIndexCapacity() - mesh->GetIndexSize() <= 0) {
            meshGroup_->emplace_back();
            mesh = &meshGroup_->back();
            mesh->Initialize(100, 100);

            mesh->SetVertexSize(2);
            mesh->SetIndexSize(2);

            addedMeshIndex_ = static_cast<int32_t>(meshGroup_->size() - 1);
        } else {
            mesh->SetVertexSize(mesh->GetVertexSize() + 2);
            mesh->SetIndexSize(mesh->GetIndexSize() + 2);
        }

        // 頂点の初期化
        vertex1Index_ = mesh->GetVertexSize() - 2;
        vertex2Index_ = mesh->GetVertexSize() - 1;

        mesh->vertexes_[vertex1Index_].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);
        mesh->vertexes_[vertex2Index_].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);

        mesh->vertexes_[vertex1Index_].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        mesh->vertexes_[vertex2Index_].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

        mesh->indexes_[vertex1Index_] = vertex1Index_;
        mesh->indexes_[vertex2Index_] = vertex2Index_;
    }

    void Undo() override {
        if (addedMeshIndex_ != -1) {
            // 新しく追加したメッシュを削除
            meshGroup_->pop_back();
        } else {
            Mesh<ColorVertexData>* mesh = &meshGroup_->back();
            mesh->SetVertexSize(mesh->GetVertexSize() - 2);
            mesh->SetIndexSize(mesh->GetIndexSize() - 2);
        }
    }

private:
    std::shared_ptr<std::vector<Mesh<ColorVertexData>>> meshGroup_;
    int32_t addedMeshIndex_; // 新しく追加したメッシュのインデックス
    int32_t vertex1Index_; // 追加した頂点1のインデックス
    int32_t vertex2Index_; // 追加した頂点2のインデックス
};

#endif // _DEBUG

LineRenderer::LineRenderer() : MeshRenderer() {
    meshGroup_->push_back(Mesh<ColorVertexData>());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(100, 100);
    mesh.SetVertexSize(0);
    mesh.SetIndexSize(0);
}

LineRenderer::LineRenderer(const std::vector<Mesh<ColorVertexData>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::LineRenderer(const std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::~LineRenderer() {}

void LineRenderer::Initialize(Entity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);
    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    transformBuff_.openData_.UpdateMatrix();
    transformBuff_.ConvertToBuffer();
}

void LineRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    std::string label;
    int32_t meshIndex = 0;
    int32_t lineIndex = 0;

    std::string startLabel;
    std::string endLabel;
    std::string startColorLabel;
    std::string endColorLabel;

    ImGui::Checkbox("LineIsStrip", &lineIsStrip_);

    transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);

    for (auto& mesh : *meshGroup_) {
        label = "Mesh[" + std::to_string(meshIndex) + "]";
        if (ImGui::TreeNode(label.c_str())) {

            for (size_t i = 0; i < mesh.vertexes_.size(); i += 2) {
                // 2つの頂点を更新
                auto& vertex1 = mesh.vertexes_[i];
                auto& vertex2 = mesh.vertexes_[i + 1];

                std::string lineLabel = "Line[" + std::to_string(lineIndex) + "]";
                ImGui::Text("%s", lineLabel.c_str());

                startLabel      = "start##" + std::to_string(lineIndex);
                endLabel        = "end##" + std::to_string(lineIndex);
                startColorLabel = "start Color##" + std::to_string(lineIndex);
                endColorLabel   = "end Color##" + std::to_string(lineIndex);

                DragGuiVectorCommand(startLabel.c_str(), vertex1.pos);
                vertex1.pos[W] = 1.f;
                DragGuiVectorCommand(endLabel.c_str(), vertex2.pos);
                vertex2.pos[W] = 1.f;
                ImGui::Spacing();
                ColorEditGuiCommand(startColorLabel, vertex1.color);
                ColorEditGuiCommand(endColorLabel, vertex2.color);

                ++lineIndex;
            }
            ImGui::TreePop();

            mesh.TransferData();
            ++meshIndex;
        }
    }

    if (ImGui::Button("AddLine")) {

        auto command = std::make_unique<AddLineCommand>(meshGroup_);
        EditorController::GetInstance()->PushCommand(std::move(command));
    }

#endif // DEBUG
}

void to_json(nlohmann::json& j, const LineRenderer& r) {
    j["isRender"]  = r.isRender_;
    j["blendMode"] = static_cast<int32_t>(r.currentBlend_);

    j["lineIsStrip"] = r.lineIsStrip_;

    // transform
    nlohmann::json transformBufferData;
    to_json(transformBufferData, r.transformBuff_.openData_);
    j["transformBufferData"] = transformBufferData;

    // mesh
    nlohmann::json meshGroupDatas = nlohmann::json::array();
    for (uint32_t meshIndex = 0; meshIndex < r.meshGroup_->size(); ++meshIndex) {
        nlohmann::json meshData;

        meshData["vertexSize"] = r.meshGroup_->at(meshIndex).GetVertexSize();
        meshData["indexSize"]  = r.meshGroup_->at(meshIndex).GetIndexSize();

        meshData["vertexes"] = nlohmann::json::array();
        for (uint32_t vertexIndex = 0; vertexIndex < r.meshGroup_->at(meshIndex).GetVertexSize(); ++vertexIndex) {
            nlohmann::json vertexData;
            vertexData["pos"]   = r.meshGroup_->at(meshIndex).vertexes_[vertexIndex].pos;
            vertexData["color"] = r.meshGroup_->at(meshIndex).vertexes_[vertexIndex].color;

            meshData["vertexes"].emplace_back(vertexData);
        }

        meshData["indexes"] = nlohmann::json::array();
        for (uint32_t indexIndex = 0; indexIndex < r.meshGroup_->at(meshIndex).GetIndexSize(); ++indexIndex) {
            nlohmann::json indexData;
            indexData = r.meshGroup_->at(meshIndex).indexes_[indexIndex];
            meshData["indexes"].emplace_back(indexData);
        }

        meshGroupDatas.emplace_back(meshData);
    }

    j["meshGroupDatas"] = meshGroupDatas;
}

void from_json(const nlohmann::json& j, LineRenderer& r) {
    j.at("isRender").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);

    j.at("lineIsStrip").get_to(r.lineIsStrip_);

    // transform
    j.at("transformBufferData").get_to(r.transformBuff_.openData_);

    // mesh
    auto& meshGroupDatas = j.at("meshGroupDatas");
    for (auto& meshData : meshGroupDatas) {
        Mesh<ColorVertexData> mesh;
        mesh.SetVertexSize(meshData.at("vertexSize"));
        mesh.SetIndexSize(meshData.at("indexSize"));
        for (auto& vertexData : meshData.at("vertexes")) {
            ColorVertexData vertex;
            vertex.pos   = vertexData.at("pos");
            vertex.color = vertexData.at("color");
            mesh.vertexes_.emplace_back(vertex);
        }
        for (auto& indexData : meshData.at("indexes")) {
            int32_t index;
            index = indexData;
            mesh.indexes_.emplace_back(index);
        }
        r.meshGroup_->emplace_back(mesh);
    }
}

void LineRenderer::Finalize() {
    for (auto& mesh : *meshGroup_) {
        mesh.Finalize();
    }
    meshGroup_.reset();
    transformBuff_.Finalize();
}

#pragma endregion
