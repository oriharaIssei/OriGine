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

    _writer.Write<std::string>("directory", directory_);
    _writer.Write<std::string>("fileName", fileName_);
}

void ModelMeshRenderer::Load(BinaryReader& _reader) {
    MeshRenderer::Load(_reader);

    _reader.Read<std::string>("directory", directory_);
    _reader.Read<std::string>("fileName", fileName_);

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

#pragma endregion

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

                isChange |= ImGui::InputFloat3(startLabel.c_str(), vertex1.pos.v);
                isChange |= ImGui::InputFloat3(endLabel.c_str(), vertex2.pos.v);
                ImGui::Spacing();
                isChange |= ImGui::ColorEdit4(startColorLabel.c_str(), vertex1.color.v);
                isChange |= ImGui::ColorEdit4(endColorLabel.c_str(), vertex2.color.v);

                ++lineIndex;
            }
            ImGui::TreePop();

            mesh.TransferData();
            ++meshIndex;
        }
    }

    if (ImGui::Button("AddLine")) {

        Mesh<ColorVertexData>* mesh = &meshGroup_->back();
        int32_t vertex1Index        = 0;
        int32_t vertex2Index        = 1;

        // 1つのラインを追加
        if (mesh->getIndexCapacity() - mesh->getIndexSize() <= 0) {
            meshGroup_->emplace_back();

            // 新しいメッシュを作成
            mesh = &meshGroup_->back();
            mesh->Initialize(100, 100);

            // 今追加した分の
            mesh->setVertexSize(2);
            mesh->setIndexSize(2);
        } else {
            mesh->setVertexSize(mesh->getVertexSize() + 2);
            mesh->setIndexSize(mesh->getIndexSize() + 2);
        }
        // 頂点の初期化
        vertex1Index = mesh->getVertexSize() - 2;
        vertex2Index = mesh->getVertexSize() - 1;

        mesh->vertexes_[vertex1Index].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);
        mesh->vertexes_[vertex2Index].pos = Vector4(0.0f, 0.0f, 0.0f, 0.f);

        mesh->vertexes_[vertex1Index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        mesh->vertexes_[vertex2Index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

        mesh->indexes_[vertex1Index] = vertex1Index;
        mesh->indexes_[vertex2Index] = vertex2Index;

        isChange = true;
    }

    return isChange;
}

void LineRenderer::Save(BinaryWriter& _writer) {
    MeshRenderer::Save(_writer);

    _writer.Write<bool>("lineIsStrip", lineIsStrip_);

    // transform
    transformBuff_->Save(_writer);

    // mesh
    int32_t meshSize = static_cast<int32_t>(meshGroup_->size());
    _writer.Write<int32_t>("meshSize", meshSize);
    std::string meshLabel;
    for (int32_t meshIndex = 0; meshIndex < meshSize; ++meshIndex) {
        meshLabel  = "Mesh_" + std::to_string(meshIndex);
        auto& mesh = (*meshGroup_)[meshIndex];

        _writer.Write<uint32_t>(meshLabel + "_vertexSize", mesh.getVertexSize());
        _writer.Write<uint32_t>(meshLabel + "_indexSize", mesh.getIndexSize());
        // vertex
        for (uint32_t vertIndex = 0; vertIndex < mesh.getVertexSize(); ++vertIndex) {
            _writer.Write<4, float>(meshLabel + "pos" + std::to_string(vertIndex), mesh.vertexes_[vertIndex].pos);
            _writer.Write<4, float>(meshLabel + "color" + std::to_string(vertIndex), mesh.vertexes_[vertIndex].color);
        }
        // index
        for (uint32_t indexNum = 0; indexNum < mesh.getIndexSize(); ++indexNum) {
            _writer.Write<uint32_t>(meshLabel + "indexData" + std::to_string(indexNum), mesh.indexes_[indexNum]);
        }
    }
}

void LineRenderer::Load(BinaryReader& _reader) {
    MeshRenderer::Load(_reader);

    _reader.Read<bool>("lineIsStrip", lineIsStrip_);

    transformBuff_->Load(_reader);

    // mesh
    int32_t meshSize = 0;
    _reader.Read<int32_t>("meshSize", meshSize);
    std::string meshLabel;
    for (int32_t meshIndex = 0; meshIndex < meshSize; ++meshIndex) {
        meshLabel           = "Mesh_" + std::to_string(meshIndex);
        auto& mesh          = (*meshGroup_)[meshIndex];
        uint32_t vertexSize = 0;
        uint32_t indexSize  = 0;
        _reader.Read<uint32_t>(meshLabel + "_vertexSize", vertexSize);
        _reader.Read<uint32_t>(meshLabel + "_indexSize", indexSize);
        mesh.setVertexSize(vertexSize);
        mesh.setIndexSize(indexSize);
        // vertex
        for (uint32_t vertIndex = 0; vertIndex < mesh.getVertexSize(); ++vertIndex) {
            ColorVertexData vertexData;
            _reader.Read<4, float>(meshLabel + "pos" + std::to_string(vertIndex), vertexData.pos);
            _reader.Read<4, float>(meshLabel + "color" + std::to_string(vertIndex), vertexData.color);
            mesh.vertexes_[vertIndex] = vertexData;
        }
        // index
        for (uint32_t indexNum = 0; indexNum < mesh.getIndexSize(); ++indexNum) {
            uint32_t indexData = 0;
            _reader.Read<uint32_t>(meshLabel + "indexData" + std::to_string(indexNum), indexData);
            mesh.indexes_[indexNum] = indexData;
        }
        mesh.TransferData();
    }
}
void LineRenderer::Finalize() {
    for (auto& mesh : *meshGroup_) {
        mesh.Finalize();
    }
    meshGroup_.reset();
    transformBuff_.Finalize();
}
