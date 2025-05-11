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
#include "module/editor/EditorGroup.h"
#include "module/editor/IEditor.h"

/// lib
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
        to_json(bufferData, r.meshMaterialBuff_[i].openData_);
        materialBufferDatas.push_back(bufferData);
    }
    j["materialBufferDatas"] = materialBufferDatas;
}

void from_json(const nlohmann::json& j, ModelMeshRenderer& r) {
    j.at("isRender").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);

    j.at("directory").get_to(r.directory_);
    j.at("fileName").get_to(r.fileName_);

    auto& transformBufferDatas = j.at("transformBufferDatas");
    for (int32_t i = 0; i < r.meshGroup_->size(); ++i) {
        Transform transform;
        transformBufferDatas[i].get_to(transform);
        r.meshTransformBuff_[i].openData_ = transform;
    }

    auto& materialBufferDatas = j.at("materialBufferDatas");
    for (int32_t i = 0; i < r.meshGroup_->size(); ++i) {
        Material material;
        materialBufferDatas[i].get_to(material);
        r.meshMaterialBuff_[i].openData_ = material;
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
    if (meshTextureNumber_.size() != meshGroup_->size()) {
        meshTextureNumber_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i]->Update();
        meshTransformBuff_[i].ConvertToBuffer();

        meshMaterialBuff_[i] = IConstantBuffer<Material>(Material());
        meshMaterialBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

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

        meshTextureNumber_[i] = 0;
    }
}

void ModelMeshRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    if (!fileName_.empty()) {
        CreateModelMeshRenderer(this, hostEntity_, directory_, fileName_);
    }

    Transform* entityTransform = getComponent<Transform>(_hostEntity);
    meshTransformBuff_.resize(meshGroup_->size());
    meshTextureNumber_.resize(meshGroup_->size());
    meshMaterialBuff_.resize(meshGroup_->size());

    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        /// ---------------------------------------------------
        // Transform
        /// ---------------------------------------------------
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        if (meshTransformBuff_[i]->parent == nullptr) {
            meshTransformBuff_[i]->parent = entityTransform;
        }

        meshTransformBuff_[i].openData_.Update();
        meshTransformBuff_[i].ConvertToBuffer();

        /// ---------------------------------------------------
        // material
        /// ---------------------------------------------------
        meshMaterialBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
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
                EditorGroup::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
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
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"obj", "gltf"})) {
            auto setDirectory = std::make_unique<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory);
            auto setName      = std::make_unique<SetterCommand<std::string>>(&fileName_, fileName);
            CommandCombo commandCombo;
            commandCombo.addCommand(std::move(setDirectory));
            commandCombo.addCommand(std::move(setName));
            commandCombo.setFuncOnAfterCommand([this]() {
                this->Finalize();
                meshGroup_ = std::make_shared<std::vector<TextureMesh>>();
                CreateModelMeshRenderer(this, this->hostEntity_, this->directory_, this->fileName_);
            });
            EditorGroup::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));

            isChange = true;
        }
    }

    ImGui::Separator();

    std::string meshName = "Mesh";
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshName = std::format("Mesh [{}]", i);
        if (ImGui::CollapsingHeader(meshName.c_str())) {
            ImGui::Indent();

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
                meshMaterialBuff_[i].openData_.DebugGui();
                meshMaterialBuff_[i].ConvertToBuffer();
                ImGui::TreePop();
            }

            ImGui::Unindent();
        }
    }
    return isChange;
#else
    return false;
#endif // _DEBUG
}

void ModelMeshRenderer::InitializeTransformBuffer(GameEntity* _hostEntity) {
    hostEntity_ = _hostEntity;
    meshTransformBuff_.resize(meshGroup_->size());
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        meshTransformBuff_[i].openData_.parent = getComponent<Transform>(_hostEntity);
        meshTransformBuff_[i].ConvertToBuffer();
    }
}

void ModelMeshRenderer::InitializeMaterialBuffer(GameEntity* _hostEntity) {
    hostEntity_ = _hostEntity;
    meshMaterialBuff_.resize(meshGroup_->size());
    for (int32_t i = 0; i < meshGroup_->size(); ++i) {
        meshMaterialBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    }
}

#pragma endregion

void CreateModelMeshRenderer(ModelMeshRenderer* _renderer, GameEntity* _hostEntity, const std::string& _directory, const std::string& _filenName) {
    _renderer->setParentTransform(getComponent<Transform>(_hostEntity));

    bool isLoaded = false;
    // -------------------- Modelの読み込み --------------------//
    auto model = ModelManager::getInstance()->Create(_directory, _filenName, [&_hostEntity, &_renderer, &isLoaded](Model* model) {
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
        };

        // メッシュグループの作成
        CreateMeshGroupFormNode(_renderer, model, &model->meshData_->rootNode);

        // Bufferの初期化
        _renderer->ResizeMaterialBuffer2MeshGroupSize();
        _renderer->ResizeTransformBuffer2MeshGroupSize();

        _renderer->InitializeTransformBuffer(_hostEntity);
        _renderer->InitializeMaterialBuffer(_hostEntity);

        for (uint32_t i = 0; i < static_cast<uint32_t>(_renderer->getMeshSize()); ++i) {
            // マテリアルの設定
            _renderer->setMaterialBuff(i, model->materialData_[i].material.openData_);
            _renderer->setTextureNumber(i, model->materialData_[i].textureNumber);
        }

        isLoaded = true;
    });

    // ロードが完了するまで待機
    while (!isLoaded) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

#pragma region "LineRenderer"
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
        if (mesh->getIndexCapacity() - mesh->getIndexSize() <= 0) {
            meshGroup_->emplace_back();
            mesh = &meshGroup_->back();
            mesh->Initialize(100, 100);

            mesh->setVertexSize(2);
            mesh->setIndexSize(2);

            addedMeshIndex_ = static_cast<int32_t>(meshGroup_->size() - 1);
        } else {
            mesh->setVertexSize(mesh->getVertexSize() + 2);
            mesh->setIndexSize(mesh->getIndexSize() + 2);
        }

        // 頂点の初期化
        vertex1Index_ = mesh->getVertexSize() - 2;
        vertex2Index_ = mesh->getVertexSize() - 1;

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
            mesh->setVertexSize(mesh->getVertexSize() - 2);
            mesh->setIndexSize(mesh->getIndexSize() - 2);
        }
    }

private:
    std::shared_ptr<std::vector<Mesh<ColorVertexData>>> meshGroup_;
    int32_t addedMeshIndex_; // 新しく追加したメッシュのインデックス
    int32_t vertex1Index_; // 追加した頂点1のインデックス
    int32_t vertex2Index_; // 追加した頂点2のインデックス
};

LineRenderer::LineRenderer() : MeshRenderer() {
    meshGroup_->push_back(Mesh<ColorVertexData>());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(100, 100);
    mesh.setVertexSize(0);
    mesh.setIndexSize(0);
}

LineRenderer::LineRenderer(const std::vector<Mesh<ColorVertexData>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::LineRenderer(const std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup)
    : MeshRenderer<Mesh<ColorVertexData>, ColorVertexData>(_meshGroup) {}

LineRenderer::~LineRenderer() {}

void LineRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);
    Transform* entityTransform = getComponent<Transform>(_hostEntity);

    // transform
    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    transformBuff_.openData_.parent = entityTransform;
    transformBuff_.openData_.Update();
    transformBuff_.ConvertToBuffer();
}

bool LineRenderer::Edit() {
    bool isChange = false;

    std::string label;
    int32_t meshIndex = 0;
    int32_t lineIndex = 0;

    std::string startLabel;
    std::string endLabel;
    std::string startColorLabel;
    std::string endColorLabel;

    isChange |= MeshRenderer::Edit();

    isChange |= ImGui::Checkbox("LineIsStrip", &lineIsStrip_);

    isChange |= transformBuff_.openData_.Edit();

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

                isChange |= DragVectorCommand(startLabel.c_str(), vertex1.pos);
                vertex1.pos[W] = 1.f;
                isChange |= DragVectorCommand(endLabel.c_str(), vertex2.pos);
                vertex2.pos[W] = 1.f;
                ImGui::Spacing();
                isChange |= ColorEditCommand(startColorLabel, vertex1.color);
                isChange |= ColorEditCommand(endColorLabel, vertex2.color);

                ++lineIndex;
            }
            ImGui::TreePop();

            mesh.TransferData();
            ++meshIndex;
        }
    }

    if (ImGui::Button("AddLine")) {

        auto command = std::make_unique<AddLineCommand>(meshGroup_);
        EditorGroup::getInstance()->pushCommand(std::move(command));

        isChange = true;
    }

    return isChange;
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

        meshData["vertexSize"] = r.meshGroup_->at(meshIndex).getVertexSize();
        meshData["indexSize"]  = r.meshGroup_->at(meshIndex).getIndexSize();

        meshData["vertexes"] = nlohmann::json::array();
        for (uint32_t vertexIndex = 0; vertexIndex < r.meshGroup_->at(meshIndex).getVertexSize(); ++vertexIndex) {
            nlohmann::json vertexData;
            vertexData["pos"]   = r.meshGroup_->at(meshIndex).vertexes_[vertexIndex].pos;
            vertexData["color"] = r.meshGroup_->at(meshIndex).vertexes_[vertexIndex].color;

            meshData["vertexes"].emplace_back(vertexData);
        }

        meshData["indexes"] = nlohmann::json::array();
        for (uint32_t indexIndex = 0; indexIndex < r.meshGroup_->at(meshIndex).getIndexSize(); ++indexIndex) {
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
        mesh.setVertexSize(meshData.at("vertexSize"));
        mesh.setIndexSize(meshData.at("indexSize"));
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
