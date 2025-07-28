#include "Primitive.h"

/// stl
#include <memory>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// directX12
#include "directX12/DxDevice.h"
// module
#include "editor/EditorController.h"
#include "editor/IEditor.h"
/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#include <imgui/imgui.h>
#endif // _DEBUG

const char* PrimitiveTypeToString(PrimitiveType _type) {
    switch (_type) {
    case PrimitiveType::Plane:
        return "Plane";
    case PrimitiveType::Ring:
        return "Ring";
    // case PrimitiveType::Circle:
    //     return "Circle";
    case PrimitiveType::Box:
        return "Box";
    // case PrimitiveType::Sphere:
    //     return "Sphere";
    // case PrimitiveType::Torus:
    //     return "Torus";
    // case PrimitiveType::Cylinder:
    //     return "Cylinder";
    // case PrimitiveType::Cone:
    //     return "Cone";
    default:
        return "Unknown";
    }
}

#pragma region "PrimitiveData"
namespace Primitive {

/// =====================================================
/// Plane
/// =====================================================
void Plane::createMesh(TextureMesh* _mesh) {
    vertexSize_ = 4; // 頂点数
    indexSize_  = 6; // インデックス数

    if ((int32_t)_mesh->getIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // XZ 平面を作成
    _mesh->setVertexData({{Vec4f(-size_[X], size_[Y], 0.0f, 1.0f), Vec2f(0.0f, 0.0f), normal_},
        {Vec4f(size_[X], size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], 0.0f), normal_},
        {Vec4f(-size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(0.0f, uv_[Y]), normal_},
        {Vec4f(size_[X], -size_[Y], 0.0f, 1.0f), Vec2f(uv_[X], uv_[Y]), normal_}});
    _mesh->setIndexData(
        {0, 1, 2,
            1, 3, 2});
    _mesh->TransferData();
}

/// =====================================================
/// Ring
/// =====================================================
void Ring::createMesh(TextureMesh* _mesh) {
    if (!_mesh->vertexes_.empty()) {
        _mesh->vertexes_.clear();
    }
    if (!_mesh->indexes_.empty()) {
        _mesh->indexes_.clear();
    }

    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(division_);

    vertexSize_ = division_ * 4; // 1つの円環は division_ * 4 頂点
    indexSize_  = division_ * 6; // 1つの円環は division_ * 6 インデックス

    if (_mesh->getIndexCapacity() < (int32_t)indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 円環の頂点を計算
    for (uint32_t i = 0; i < division_; ++i) {

        float sin     = std::sin(radianPerDivide * static_cast<float>(i));
        float cos     = std::cos(radianPerDivide * static_cast<float>(i));
        float sinNext = std::sin(radianPerDivide * static_cast<float>(i + 1));
        float cosNext = std::cos(radianPerDivide * static_cast<float>(i + 1));
        float u       = static_cast<float>(i) / static_cast<float>(division_);
        float uNext   = static_cast<float>(i + 1) / static_cast<float>(division_);

        // Vertex
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * outerRadius_, cos * outerRadius_, 0.f, 1.f), Vec2f(u, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * outerRadius_, cosNext * outerRadius_, 0.f, 1.f), Vec2f(uNext, 0.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sin * innerRadius_, cos * innerRadius_, 0.f, 1.f), Vec2f(u, 1.f), Vector3f(0.f, 0.f, -1.f)));
        _mesh->vertexes_.emplace_back(TextureVertexData(Vector4f(-sinNext * innerRadius_, cosNext * innerRadius_, 0.f, 1.f), Vec2f(uNext, 1.f), Vector3f(0.f, 0.f, -1.f)));

        // Index
        uint32_t startIndex = i * 4;
        _mesh->indexes_.emplace_back(startIndex);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 3);
    }

    _mesh->TransferData();
}

/// =====================================================
/// Box
/// =====================================================
void Box::createMesh(TextureMesh* _mesh) {
    if ((int32_t)_mesh->getIndexCapacity() < indexSize_) {
        // 必要なら Finalize
        if (_mesh->getVertexBuffer().getResource()) {
            _mesh->Finalize();
        }
        _mesh->Initialize(vertexSize_, indexSize_);
        _mesh->vertexes_.clear();
        _mesh->indexes_.clear();
    }

    // 頂点データを設定
    std::vector<TextureMesh::VertexType> vertices;
    vertices.reserve(vertexSize_);

    Vec3f halfSize = size_ * 0.5f;
    Vec3f minPos   = -halfSize;
    Vec3f maxPos   = halfSize;

    // 立方体の8つの頂点を定義
    // left bottom front
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], minPos[Y], minPos[Z], 1.0f), Vec2f(0.0f, 0.0f), Vec3f(-1.0f, -1.0f, -1.0f)));
    // right bottom front
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], minPos[Y], minPos[Z], 1.0f), Vec2f(1.0f, 0.0f), Vec3f(1.0f, -1.0f, -1.0f)));
    // left top front
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], maxPos[Y], minPos[Z], 1.0f), Vec2f(0.0f, 1.0f), Vec3f(-1.0f, 1.0f, -1.0f)));
    // right top front
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], maxPos[Y], minPos[Z], 1.0f), Vec2f(1.0f, 1.0f), Vec3f(1.0f, 1.0f, -1.0f)));
    // left bottom back
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], minPos[Y], maxPos[Z], 1.0f), Vec2f(0.0f, 0.0f), Vec3f(-1.0f, -1.0f, 1.0f)));
    // right bottom back
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], minPos[Y], maxPos[Z], 1.0f), Vec2f(1.0f, 0.0f), Vec3f(1.0f, -1.0f, 1.0f)));
    // left top back
    vertices.emplace_back(TextureVertexData(Vec4f(minPos[X], maxPos[Y], maxPos[Z], 1.0f), Vec2f(0.0f, 1.0f), Vec3f(-1.0f, 1.0f, 1.0f)));
    // right top back
    vertices.emplace_back(TextureVertexData(Vec4f(maxPos[X], maxPos[Y], maxPos[Z], 1.0f), Vec2f(1.0f, 1.0f), Vec3f(1.0f, 1.0f, 1.0f)));
    _mesh->setVertexData(vertices);

    // インデックスデータを設定
    std::vector<uint32_t> indices;
    indices.reserve(indexSize_);
    // 前面
    indices.insert(indices.end(), {0, 1, 2, 1, 3, 2});
    // 背面
    indices.insert(indices.end(), {4, 6, 5, 5, 6, 7});
    // 左面
    indices.insert(indices.end(), {0, 2, 4, 2, 6, 4});
    // 右面
    indices.insert(indices.end(), {1, 5, 3, 3, 5, 7});
    // 上面
    indices.insert(indices.end(), {2, 3, 6, 3, 7, 6});
    // 下面
    indices.insert(indices.end(), {0, 4, 1, 1, 4, 5});
    _mesh->setIndexData(indices);

    _mesh->TransferData();
}

/// =====================================================
/// Sphere
/// ====================================================
void Sphere::createMesh(TextureMesh* _mesh) {
    // パラメータ
    const uint32_t latitudeDiv  = divisionLatitude_; // 緯度分割数
    const uint32_t longitudeDiv = divisionLongitude_; // 経度分割数
    const float radius          = radius_;

    // 頂点・インデックスバッファ初期化
    _mesh->vertexes_.clear();
    _mesh->indexes_.clear();

    // 頂点生成
    for (uint32_t lat = 0; lat <= latitudeDiv; ++lat) {
        float theta    = float(lat) * std::numbers::pi_v<float> / float(latitudeDiv); // 0 ~ pi
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (uint32_t lon = 0; lon <= longitudeDiv; ++lon) {
            float phi    = float(lon) * 2.0f * std::numbers::pi_v<float> / float(longitudeDiv); // 0 ~ 2pi
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Vec3f pos = {
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi};
            Vec3f normal = pos.normalize();
            Vec2f uv     = {
                float(lon) / float(longitudeDiv),
                float(lat) / float(latitudeDiv)};

            _mesh->vertexes_.emplace_back(TextureVertexData(Vec4f(pos, 1.0f), uv, normal));
        }
    }

    // インデックス生成
    for (uint32_t lat = 0; lat < latitudeDiv; ++lat) {
        for (uint32_t lon = 0; lon < longitudeDiv; ++lon) {
            uint32_t current = lat * (longitudeDiv + 1) + lon;
            uint32_t next    = current + longitudeDiv + 1;

            // 2つの三角形で四角形を構成
            _mesh->indexes_.emplace_back(current);
            _mesh->indexes_.emplace_back(next);
            _mesh->indexes_.emplace_back(current + 1);

            _mesh->indexes_.emplace_back(current + 1);
            _mesh->indexes_.emplace_back(next);
            _mesh->indexes_.emplace_back(next + 1);
        }
    }

    // 頂点数・インデックス数を記録
    vertexSize_ = static_cast<int32_t>(_mesh->vertexes_.size());
    indexSize_  = static_cast<int32_t>(_mesh->indexes_.size());

    _mesh->TransferData();
}

}
#pragma endregion

#pragma region "PrimitiveRenderer"

std::shared_ptr<PrimitiveMeshRendererBase> CreatePrimitiveRenderer(PrimitiveType _type) {
    switch (_type) {
    case PrimitiveType::Plane:
        return std::make_shared<PlaneRenderer>();
        break;
    case PrimitiveType::Ring:
        return std::make_shared<RingRenderer>();
        break;
    case PrimitiveType::Box:
        return std::make_shared<BoxRenderer>();
        break;
    case PrimitiveType::Sphere:
        return std::make_shared<SphereRenderer>();
        break;

    default:
        break;
    }
    return nullptr;
}

/// =====================================================
/// PlaneRenderer
/// =====================================================

void PlaneRenderer::Initialize(GameEntity* /*_hostEntity*/) {

    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(4, 6);

    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // create _mesh
    createMesh(&mesh);

    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void PlaneRenderer::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());

    std::string label = "LoadTexture##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));

            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);

            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);
        ImGui::TreePop();
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        materialBuff_.openData_.DebugGui(_parentLabel);
        materialBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void to_json(nlohmann::json& j, const PlaneRenderer& r) {
    j["isRenderer"]       = r.isRender_;
    j["blendMode"]        = static_cast<int32_t>(r.currentBlend_);
    j["textureDirectory"] = r.textureDirectory_;
    j["textureFileName"]  = r.textureFileName_;
    to_json(j["transform"], r.transformBuff_.openData_);
    to_json(j["material"], r.materialBuff_.openData_);
    j["material"] = r.materialBuff_.openData_;
}

void from_json(const nlohmann::json& j, PlaneRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(r.textureDirectory_);
    j.at("textureFileName").get_to(r.textureFileName_);
    from_json(j.at("transform"), r.transformBuff_.openData_);
    from_json(j.at("material"), r.materialBuff_.openData_);
    j.at("material").get_to(r.materialBuff_.openData_);
}

/// =====================================================
// RingRenderer
/// =====================================================

void RingRenderer::Initialize(GameEntity* _hostEntity) {
    _hostEntity;
    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();

    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // create _mesh
    createMesh(&mesh);

    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void RingRenderer::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            blendIndex++;
        }
        ImGui::EndCombo();
    }

    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Shape");
    ImGui::Spacing();

    // shape
    int32_t division = primitive_.getDivision();
    label            = "Division##" + _parentLabel;
    DragGuiCommand<int32_t>(label, division, 1, 1, 1000, "%d", [this](int32_t* _value) {
        primitive_.setDivision(static_cast<uint32_t>(*_value));
        createMesh(&meshGroup_->back());
    });
    primitive_.setDivision(static_cast<uint32_t>(division));

    float innerRadius = primitive_.getInnerRadius();
    label             = "InnerRadius##" + _parentLabel;
    DragGuiCommand<float>(label, innerRadius, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.setInnerRadius(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setInnerRadius(innerRadius);

    float outerRadius = primitive_.getOuterRadius();
    label             = "OuterRadius##" + _parentLabel;
    DragGuiCommand<float>(label, outerRadius, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.setOuterRadius(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setOuterRadius(outerRadius);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // buffer Datas
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);
        transformBuff_.ConvertToBuffer();

        ImGui::TreePop();
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        materialBuff_.openData_.DebugGui(_parentLabel);
        materialBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void to_json(nlohmann::json& j, const RingRenderer& r) {
    j["isRenderer"]       = r.isRender_;
    j["blendMode"]        = static_cast<int32_t>(r.currentBlend_);
    j["textureDirectory"] = r.textureDirectory_;
    j["textureFileName"]  = r.textureFileName_;
    to_json(j["transform"], r.transformBuff_.openData_);
    to_json(j["material"], r.materialBuff_.openData_);
    j["material"] = r.materialBuff_.openData_;

    j["InnerRadius"] = r.primitive_.getInnerRadius();
    j["OuterRadius"] = r.primitive_.getOuterRadius();
    j["division"]    = r.primitive_.getDivision();
}

void from_json(const nlohmann::json& j, RingRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(r.textureDirectory_);
    j.at("textureFileName").get_to(r.textureFileName_);
    from_json(j.at("transform"), r.transformBuff_.openData_);
    from_json(j.at("material"), r.materialBuff_.openData_);
    j.at("material").get_to(r.materialBuff_.openData_);

    float innerRadius = 0.f;
    float outerRadius = 0.f;
    int32_t divi      = 0;
    j.at("InnerRadius").get_to(innerRadius);
    j.at("OuterRadius").get_to(outerRadius);
    j.at("division").get_to(divi);
    r.primitive_.setInnerRadius(innerRadius);
    r.primitive_.setOuterRadius(outerRadius);
    r.primitive_.setDivision(divi);
}

/// =====================================================
// BoxRenderer
/// =====================================================

void BoxRenderer::Initialize(GameEntity* _hostEntity) {
    _hostEntity;
    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();

    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // create _mesh
    createMesh(&mesh);

    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void BoxRenderer::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            ++blendIndex;
        }
        ImGui::EndCombo();
    }

    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // shape
    Vec3f size = primitive_.getSize();
    label      = "Size##" + _parentLabel;
    DragGuiVectorCommand<3, float>(label.c_str(), size, 0.01f, 0.01f, {}, "%.3f", [this](Vec<3, float>* _value) {
        primitive_.setSize(*_value);
        createMesh(&meshGroup_->back());
    });

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // buffer Datas
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);

        transformBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        materialBuff_.openData_.DebugGui(_parentLabel);
        materialBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void to_json(nlohmann::json& j, const BoxRenderer& r) {
    j["isRenderer"]       = r.isRender_;
    j["blendMode"]        = static_cast<int32_t>(r.currentBlend_);
    j["textureDirectory"] = r.textureDirectory_;
    j["textureFileName"]  = r.textureFileName_;
    to_json(j["transform"], r.transformBuff_.openData_);
    to_json(j["material"], r.materialBuff_.openData_);
    j["material"] = r.materialBuff_.openData_;

    j["size"] = r.primitive_.getSize();
}

void from_json(const nlohmann::json& j, BoxRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(r.textureDirectory_);
    j.at("textureFileName").get_to(r.textureFileName_);
    from_json(j.at("transform"), r.transformBuff_.openData_);
    from_json(j.at("material"), r.materialBuff_.openData_);
    j.at("material").get_to(r.materialBuff_.openData_);

    Vec3f size = {1.0f, 1.0f, 1.0f};
    j.at("size").get_to(size);
    r.primitive_.setSize(size);
}

/// =====================================================
// SphereRenderer
/// =====================================================

void SphereRenderer::Initialize(GameEntity* _hostEntity) {
    _hostEntity;
    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }
    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();
    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    // create _mesh
    createMesh(&mesh);
    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void SphereRenderer::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];
            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }
            blendIndex++;
        }
        ImGui::EndCombo();
    }
    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    // shape
    int32_t divisionLatitude  = primitive_.getDivisionLatitude();
    int32_t divisionLongitude = primitive_.getDivisionLongitude();
    label                     = "Division Latitude##" + _parentLabel;
    DragGuiCommand<int32_t>(label.c_str(), divisionLatitude, 1, 1, 1000, "%d", [this](int32_t* _value) {
        primitive_.setDivisionLatitude(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setDivisionLatitude(divisionLatitude);
    label = "Division Longitude##" + _parentLabel;
    DragGuiCommand<int32_t>(label.c_str(), divisionLongitude, 1, 1, 1000, "%d", [this](int32_t* _value) {
        primitive_.setDivisionLongitude(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setDivisionLongitude(divisionLongitude);
    float radius = primitive_.getRadius();
    label        = "Radius##" + _parentLabel;
    DragGuiCommand<float>(label.c_str(), radius, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.setRadius(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setRadius(radius);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    // buffer Datas
   label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);
        transformBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        materialBuff_.openData_.DebugGui(_parentLabel);
        materialBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }
#endif // _DEBUG
}

void to_json(nlohmann::json& j, const SphereRenderer& r) {
    j["isRenderer"]       = r.isRender_;
    j["blendMode"]        = static_cast<int32_t>(r.currentBlend_);
    j["textureDirectory"] = r.textureDirectory_;
    j["textureFileName"]  = r.textureFileName_;
    to_json(j["transform"], r.transformBuff_.openData_);
    to_json(j["material"], r.materialBuff_.openData_);
    j["material"]          = r.materialBuff_.openData_;
    j["radius"]            = r.primitive_.getRadius();
    j["divisionLatitude"]  = r.primitive_.getDivisionLatitude();
    j["divisionLongitude"] = r.primitive_.getDivisionLongitude();
}
void from_json(const nlohmann::json& j, SphereRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(r.textureDirectory_);
    j.at("textureFileName").get_to(r.textureFileName_);
    from_json(j.at("transform"), r.transformBuff_.openData_);
    from_json(j.at("material"), r.materialBuff_.openData_);
    j.at("material").get_to(r.materialBuff_.openData_);
    float radius              = 1.0f;
    int32_t divisionLatitude  = 10;
    int32_t divisionLongitude = 10;
    j.at("radius").get_to(radius);
    j.at("divisionLatitude").get_to(divisionLatitude);
    j.at("divisionLongitude").get_to(divisionLongitude);
    r.primitive_.setRadius(radius);
    r.primitive_.setDivisionLatitude(divisionLatitude);
    r.primitive_.setDivisionLongitude(divisionLongitude);
}

#pragma endregion
