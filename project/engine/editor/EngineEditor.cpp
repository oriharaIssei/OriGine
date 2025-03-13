#include "EngineEditor.h"

#ifdef _DEBUG
/// engine
// module
#include "camera/CameraManager.h"
#include "component/material/light/LightManager.h"
#include "component/material/Material.h"
#include "sceneManager/SceneManager.h"
// scene
#include "iScene/IScene.h"
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
    if (isActive_) {
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
                    ImGui::Checkbox("IsEdit", &isActive_);
                    for (auto& [name, editor] : editors_) {
                        ImGui::Checkbox(name.c_str(), &editorActive_[editor.get()]);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        CameraManager::getInstance()->DebugUpdate();

        for (auto& [name, editor] : editors_) {
            if (editorActive_[editor.get()]) {
                editor->Update();
            }
        }
    } else {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("EndDebug")) {
                    isActive_ = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
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
