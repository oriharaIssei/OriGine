#include "SkyboxRenderer.h"

/// engine
#define ENGINE_ECS
#define ENGINE_EDITOR
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "texture/TextureManager.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#include <imgui/imgui.h>
#endif // _DEBUG

void SkyboxRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    /// mesh
    meshGroup_->push_back(Mesh<SkyboxVertex>());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(12, 24);

    // 右
    mesh.vertexes_[0].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[1].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[2].position = {1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[3].position = {1.f, -1.f, -1.f, 1.f};
    // 左
    mesh.vertexes_[4].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[5].position = {-1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[6].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[7].position = {-1.f, -1.f, 1.f, 1.f};
    // 前
    mesh.vertexes_[8].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[9].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[10].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[11].position = {1.f, -1.f, 1.f, 1.f};
    // 後ろ
    mesh.vertexes_[12].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[13].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[14].position = {1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[15].position = {-1.f, -1.f, -1.f, 1.f};
    // 上
    mesh.vertexes_[16].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[17].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[18].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[19].position = {1.f, 1.f, 1.f, 1.f};
    // 下
    mesh.vertexes_[20].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[21].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[22].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[23].position = {-1.f, -1.f, -1.f, 1.f};

    mesh.indexes_ = {
        0,
        1,
        2,
        2,
        1,
        3, // 右
        4,
        5,
        6,
        6,
        5,
        7, // 左
        8,
        9,
        10,
        10,
        9,
        11, // 前
        12,
        13,
        14,
        14,
        13,
        15, // 後ろ
        16,
        17,
        18,
        18,
        17,
        19, // 上
        20,
        21,
        22,
        22,
        21,
        23, // 下
    };

    if (!filePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(filePath_);
    }

    transformBuff_->Initialize(_hostEntity);
    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

bool SkyboxRenderer::Edit() {
#ifdef _DEBUG

    bool isChange = false;
    ImGui::Text("FilePath : %s", filePath_.c_str());
    if (ImGui::Button("load file")) {
        std::string directory;
        std::string filename;
        myfs::selectFileDialog(kApplicationResourceDirectory, directory, filename, {"dds"});
        if (!filename.empty()) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_unique<SetterCommand<std::string>>(&filePath_, kApplicationResourceDirectory + "/" + directory + "filename"));
            commandCombo->setFuncOnAfterCommand(
                [this]() {
                    textureIndex_ = TextureManager::LoadTexture(filePath_);
                },
                true);
            EditorGroup::getInstance()->pushCommand(std::move(commandCombo));
            isChange = true;
        }
    }

    isChange |= DragGuiVectorCommand("Color", materialBuff_.openData_.color);

    return isChange;
#else
    return false;
#endif // _DEBUG
}

void to_json(nlohmann::json& j, const SkyboxRenderer& c) {
    j["filePath"]      = c.filePath_;
    j["transformBuff"] = c.transformBuff_.openData_;
    j["materialBuff"]  = c.materialBuff_.openData_;
}

void from_json(const nlohmann::json& j, SkyboxRenderer& c) {
    j.at("filePath").get_to(c.filePath_);
    j.at("transformBuff").get_to(c.transformBuff_.openData_);
    j.at("materialBuff").get_to(c.materialBuff_.openData_);
}

void to_json(nlohmann::json& j, const SkyboxMaterial& c) {
    j["color"] = c.color;
}

void from_json(const nlohmann::json& j, SkyboxMaterial& c) {
    j.at("color").get_to(c.color);
}
