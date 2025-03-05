#include "EngineEditor.h"

#ifdef _DEBUG
/// engine
// module
#include "../application/scene/manager/SceneManager.h"
#include "camera/CameraManager.h"
#include "component/material/light/LightManager.h"
#include "component/material/Material.h"
// scene
#include "../application/scene/IScene.h"
// externals
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

EngineEditor::EngineEditor() {
    // Constructor implementation
}

EngineEditor::~EngineEditor() {
    // Destructor implementation
}

EngineEditor* EngineEditor::getInstance() {
    static EngineEditor instance;
    return &instance;
}

void EngineEditor::Update() {
    // MainMenuBar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::BeginMenu("Change")) {
                SceneManager* sceneManager = SceneManager::getInstance();
                for (auto& [name, index] : sceneManager->sceneIndexs_) {
                    if (ImGui::MenuItem(name.c_str())) {
                        sceneManager->changeScene(name);
                        break;
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Save")) {
                SceneManager::getInstance()->getCurrentScene()->SaveSceneEntity();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Editors")) {
            if (ImGui::BeginMenu("ActiveState")) {
                for (auto& [name, editor] : editors_) {
                    ImGui::Checkbox(name.c_str(), &editorActive_[editor.get()]);
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("IsDebug")) {
                ImGui::Checkbox("IsDebug", &isActive_);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (!isActive_) {
        return;
    }
    for (auto& [name, editor] : editors_) {
        if (editorActive_[editor.get()]) {
            editor->Update();
        }
    }
}

#pragma region "アクセッサー"
void EngineEditor::addEditor(const std::string& name, std::unique_ptr<IEditor>&& editor) {
    editors_[name]                      = std::move(editor);
    editorActive_[editors_[name].get()] = false;
}

#pragma endregion

#endif // _DEBUG
