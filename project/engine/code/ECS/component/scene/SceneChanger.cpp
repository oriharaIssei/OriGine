#include "SceneChanger.h"

/// engine
#define ENGINE_SCENE
#define RESOURCE_DIRECTORY
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "EngineInclude.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
// imgui
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

SceneChanger::SceneChanger() {}
SceneChanger::~SceneChanger() {}

using namespace OriGine;

void SceneChanger::Initialize(Scene* /*_scene*/, EntityHandle /* _handle */) {}

void SceneChanger::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    ImGui::Text("Next Scene Name :");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##NextSceneName", nextSceneName_.c_str())) {
        for (const auto& [directory, sceneName] : myfs::SearchFile(kApplicationResourceDirectory + "/scene/", "json")) {
            bool isSelected = nextSceneName_ == sceneName;
            if (ImGui::Selectable(sceneName.c_str(), isSelected)) {

                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<std::string>>(&nextSceneName_, sceneName));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

#endif // _DEBUG
}

void SceneChanger::Finalize() {}

void to_json(nlohmann::json& j, const SceneChanger& r) {
    j["nextSceneName"] = r.nextSceneName_;
}

void from_json(const nlohmann::json& j, SceneChanger& r) {
    j.at("nextSceneName").get_to(r.nextSceneName_);
}
