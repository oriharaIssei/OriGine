#include "SceneEditorMenus.h"

#ifdef _DEBUG

/// engine
#include "scene/SceneJsonRegistry.h"

/// util
#include "myGui/MyGui.h"
#include "util/myFileSystem/MyFileSystem.h"

using namespace OriGine;

namespace {
static const std::string sceneFolderPath = kApplicationResourceDirectory + "/" + kSceneJsonFolder;
}

SceneEditorFileMenu::SceneEditorFileMenu(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Menu("File") {}
SceneEditorFileMenu::~SceneEditorFileMenu() {}
void SceneEditorFileMenu::Initialize() {
    AddMenuItem(std::make_unique<SaveMenuItem>(this));
    AddMenuItem(std::make_unique<LoadMenuItem>(this));
    AddMenuItem(std::make_unique<CreateMenuItem>(this));
}
void SceneEditorFileMenu::Finalize() {
    for (auto& [name, item] : menuItems_) {
        item->Finalize();
    }
    menuItems_.clear();
}

SaveMenuItem::SaveMenuItem(SceneEditorFileMenu* _parent)
    : Editor::MenuItem("Save"), parentMenu_(_parent) {}
SaveMenuItem::~SaveMenuItem() {}

void SaveMenuItem::Initialize() {}

void SaveMenuItem::DrawGui() {
    bool isSelect = false;

    if (ImGui::MenuItem(name_.c_str(), "ctl + s", &isSelect)) {
        auto* currentScene = parentMenu_->GetParentWindow()->GetCurrentScene();
        LOG_DEBUG("SaveMenuItem : Saving scene '{}'.", currentScene->GetName());
        SceneJsonRegistry::GetInstance()->SaveScene(currentScene, sceneFolderPath);
    }

    isSelected_.Set(isSelect);
}

void SaveMenuItem::Finalize() {}

LoadMenuItem::LoadMenuItem(SceneEditorFileMenu* _parent)
    : Editor::MenuItem("Load"), parentMenu_(_parent) {}

LoadMenuItem::~LoadMenuItem() {}

void LoadMenuItem::Initialize() {}

void LoadMenuItem::DrawGui() {
    bool isSelect = false;
    if (ImGui::MenuItem(name_.c_str(), "ctl + o", &isSelect)) {
        // シーンのロード処理
        std::string directory, filename;
        if (!myfs::SelectFileDialog(sceneFolderPath, directory, filename, {"json"}, true)) {
            return;
        }

        SceneEditorWindow* sceneEditorWindow = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
        sceneEditorWindow->ChangeScene(filename);
    }
    isSelected_.Set(isSelect);
}

void LoadMenuItem::Finalize() {
    loadScene_ = nullptr; // ロードするシーンへのポインタをクリア
}

CreateMenuItem::CreateMenuItem(SceneEditorFileMenu* _parent)
    : Editor::MenuItem(nameof<CreateMenuItem>()), parentMenu_(_parent) {}

CreateMenuItem::~CreateMenuItem() {}

void CreateMenuItem::Initialize() {}

void CreateMenuItem::DrawGui() {
    bool isSelect = false;
    if (ImGui::BeginMenu("Create NewScene")) {
        ImGui::InputText("New Scene Name", &newSceneName_);

        if (ImGui::Button("Create")) {
            auto scenes = myfs::SearchFile(kApplicationResourceDirectory + "/" + kSceneJsonFolder, {"json"});

            auto it = std::find_if(
                scenes.begin(), scenes.end(),
                [this](const std::pair<std::string, std::string>& scene) {
                    return scene.second == newSceneName_;
                });

            if (it != scenes.end()) {
                LOG_ERROR("Scene with name '{}' already exists.", newSceneName_);
                newSceneName_ = "";
                ImGui::OpenPopup("Scene Exists");
            } else {
                auto currentScene = parentMenu_->GetParentWindow()->GetCurrentScene();

                SceneJsonRegistry::GetInstance()->SaveScene(currentScene, sceneFolderPath);

                auto newScene = std::make_unique<Scene>(newSceneName_);
                newScene->Initialize();
                parentMenu_->GetParentWindow()->ChangeScene(newSceneName_);

                // 初期化
                OriGine::EditorController::GetInstance()->ClearCommandHistory();
                newSceneName_ = "";
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginPopup("Scene Exists")) {
        ImGui::Text("Scene with this name already exists.");
        ImGui::Separator();
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    isSelected_.Set(isSelect);
}

void CreateMenuItem::Finalize() {}

#endif // _DEBUG
