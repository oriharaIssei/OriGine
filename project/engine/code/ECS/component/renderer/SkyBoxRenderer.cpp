#include "SkyboxRenderer.h"

/// engine
#define ENGINE_ECS
#define ENGINE_EDITOR
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "texture/TextureManager.h"
// directX12
#include "directX12/DxDevice.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SkyboxRenderer::Initialize(Scene* _scene, EntityHandle _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

    isRender_ = true;

    /// mesh
    meshGroup_->push_back(Mesh<SkyboxVertex>());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(8, 36);

    // 前
    mesh.vertexes_[0].position = {-1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[1].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[2].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[3].position = {1.f, -1.f, 1.f, 1.f};
    // 後ろ
    mesh.vertexes_[4].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[5].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[6].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[7].position = {1.f, -1.f, -1.f, 1.f};

    mesh.indexes_ = {
        // 前
        0,
        1,
        2,
        2,
        1,
        3,
        // 後ろ
        4,
        6,
        5,
        7,
        5,
        6,
        // 右
        1,
        5,
        3,
        3,
        5,
        7,
        // 左
        0,
        2,
        4,
        6,
        4,
        2,
        // 上
        4,
        5,
        0,
        0,
        5,
        1,
        // 下
        2,
        3,
        6,
        7,
        6,
        3,
    };

    mesh.TransferData();

    if (!filePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(filePath_);
    }

    transformBuff_->Initialize(_scene, _hostEntity);
    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SkyboxRenderer::Edit(Scene* /*_scene*/, EntityHandle /* _entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    ImGui::Text("FilePath : %s", filePath_.c_str());
    std::string label = "Load##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string filename;
        myfs::SelectFileDialog(kApplicationResourceDirectory, directory, filename, {"dds"});
        if (!filename.empty()) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::make_unique<SetterCommand<std::string>>(&filePath_, kApplicationResourceDirectory + "/" + directory + "/" + filename));
            commandCombo->SetFuncOnAfterCommand(
                [this]() {
                    TextureManager::LoadTexture(filePath_, [this](uint32_t _newTextureIndex) {
                        textureIndex_ = _newTextureIndex;
                    });
                },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ColorEditGuiCommand("Color##" + _parentLabel, materialBuff_.openData_.color);
#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const SkyboxRenderer& _comp) {
    _j["filePath"]      = _comp.filePath_;
    _j["transformBuff"] = _comp.transformBuff_.openData_;
    _j["materialBuff"]  = _comp.materialBuff_.openData_;
}

void OriGine::from_json(const nlohmann::json& _j, SkyboxRenderer& _comp) {
    _j.at("filePath").get_to(_comp.filePath_);
    _j.at("transformBuff").get_to(_comp.transformBuff_.openData_);
    _j.at("materialBuff").get_to(_comp.materialBuff_.openData_);
}

void OriGine::to_json(nlohmann::json& _j, const SkyboxMaterial& _comp) {
    _j["color"] = _comp.color;
}

void OriGine::from_json(const nlohmann::json& _j, SkyboxMaterial& _comp) {
    _j.at("color").get_to(_comp.color);
}
