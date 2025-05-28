#include "Primitive.h"

/// stl
#include <memory>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "module/editor/EditorGroup.h"
#include "module/editor/IEditor.h"

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
    // case PrimitiveType::Box:
    //     return "Box";
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

    if ((int32_t)_mesh->getIndexCapacity() < indexSize_) {
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
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 2);
        _mesh->indexes_.emplace_back(startIndex + 1);
        _mesh->indexes_.emplace_back(startIndex + 3);
        _mesh->indexes_.emplace_back(startIndex + 2);
    }

    _mesh->TransferData();
}

#pragma endregion

#pragma region "PrimitiveRenderer"

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

bool PlaneRenderer::Edit() {
#ifdef _DEBUG
    bool isEdit = false;

    MeshRenderer::Edit();

    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    if (ImGui::Button("LoadTexture")) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));

            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);

            EditorGroup::getInstance()->pushCommand(std::move(commandCombo));

            isEdit = true;
        }
    }

    if (ImGui::TreeNode("Transform")) {
        if (transformBuff_.openData_.Edit()) {
            transformBuff_.ConvertToBuffer();
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Material")) {
        materialBuff_.openData_.DebugGui();
        ImGui::TreePop();
    }

    return isEdit;
#else
    return false;
#endif // _DEBUG
}

std::shared_ptr<PrimitiveMeshRendererBase> CreatePrimitiveRenderer(PrimitiveType _type) {
    if (_type == PrimitiveType::Plane) {
        return std::make_shared<PlaneRenderer>();
    } else if (_type == PrimitiveType::Ring) {
        return std::make_shared<RingRenderer>();
    }
    return std::shared_ptr<PrimitiveMeshRendererBase>();
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

bool RingRenderer::Edit() {
    bool isEdit = false;

#ifdef _DEBUG
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
                isEdit = true;
                break;
            }

            blendIndex++;
        }
        ImGui::EndCombo();
    }

    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    if (ImGui::Button("LoadTexture")) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorGroup::getInstance()->pushCommand(std::move(commandCombo));
            isEdit = true;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // shape
    int32_t division = primitive_.getDivision();
    isEdit |= DragGuiCommand<int32_t>("Division", division, 1, 1, 1000, "%d", [this](int32_t* _value) {
        primitive_.setDivision(static_cast<uint32_t>(*_value));
        createMesh(&meshGroup_->back());
    });
    primitive_.setDivision(static_cast<uint32_t>(division));

    float innerRadius = primitive_.getInnerRadius();
    isEdit |= DragGuiCommand<float>("inner Radius", innerRadius, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.setInnerRadius(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setInnerRadius(innerRadius);

    float outerRadius = primitive_.getOuterRadius();
    isEdit |= DragGuiCommand<float>("outer Radius", outerRadius, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.setOuterRadius(*_value);
        createMesh(&meshGroup_->back());
    });
    primitive_.setOuterRadius(outerRadius);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // buffer Datas
    if (ImGui::TreeNode("Transform")) {
        if (transformBuff_.openData_.Edit()) {
            transformBuff_.ConvertToBuffer();
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Material")) {
        materialBuff_.openData_.DebugGui();
        ImGui::TreePop();
    }

#endif // _DEBUG

    return isEdit;
};

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
    j.at("InnerRadius").get_to(innerRadius);
    j.at("OuterRadius").get_to(outerRadius);
    r.primitive_.setInnerRadius(innerRadius);
    r.primitive_.setOuterRadius(outerRadius);
}

#pragma endregion
