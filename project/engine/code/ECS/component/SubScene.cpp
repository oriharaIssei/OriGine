#include "SubScene.h"

#ifdef _DEBUG
/// engine
#define RESOURCE_DIRECTORY
#include <EngineInclude.h>

/// utils
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

SubScene::SubScene() {}
SubScene::~SubScene() {}

void SubScene::Initialize(Entity* /*_entity*/) {
    if (!sceneName_.empty() && isActive_) {
        Load(sceneName_);
    }
}

void SubScene::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive##" + _parentLabel, isActive_);

    std::string label = "SceneName##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), sceneName_.c_str())) {
        std::list<std::pair<std::string, std::string>> sceneList = myfs::searchFile(kApplicationResourceDirectory + "/scene", "json");
        for (const auto& scene : sceneList) {
            bool isSelected = (sceneName_ == scene.second);
            if (ImGui::Selectable(scene.second.c_str(), isSelected)) {
                auto command = std::make_unique<SetterCommand<std::string>>(&sceneName_, scene.second, [this](std::string* _newScene) {
                    if (this) {
                        Unload();
                        Load(*_newScene);
                    }
                });
                EditorController::GetInstance()->PushCommand(std::move(command));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
#endif // _DEBUG
}

void SubScene::Finalize() {
    Unload();
}

void SubScene::Activate() {
    isActive_ = true;
    if (!subScene_) {
        Load(sceneName_);
    }
}

void SubScene::Deactivate() {
    isActive_ = false;
    Unload();
}

void SubScene::Load(const std::string& _sceneName) {
    sceneName_ = _sceneName;
    subScene_  = std::make_unique<Scene>(sceneName_);
    subScene_->Initialize();
}
void SubScene::Unload() {
    if (subScene_) {
        subScene_->Finalize();
        subScene_.reset();
    }
}

void to_json(nlohmann::json& j, const SubScene& scene) {
    j = nlohmann::json{
        {"isActive", scene.isActive_},
        {"sceneName", scene.sceneName_},
    };
}
void from_json(const nlohmann::json& j, SubScene& scene) {
    j.at("isActive").get_to(scene.isActive_);
    j.at("sceneName").get_to(scene.sceneName_);
}
