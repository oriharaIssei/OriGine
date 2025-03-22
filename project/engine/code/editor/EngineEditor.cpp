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
// lib
#include "input/Input.h"

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

void EngineEditor::Initialize() {
    for (auto& [editorName, editor] : editors_) {
        editor->Initialize();
    }
}

void EngineEditor::Update() {
    // Editor が Active のとき
    if (isActive_) {
        ///=================================================================================================
        // MainMenuBar
        ///=================================================================================================
        if (ImGui::BeginMainMenuBar()) {
            /// ------------------------------------------------------------------------------------------------
            // Scene
            /// ------------------------------------------------------------------------------------------------
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

            /// ------------------------------------------------------------------------------------------------
            // Editors
            /// ------------------------------------------------------------------------------------------------
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

            /// ------------------------------------------------------------------------------------------------
            // Debug
            /// ------------------------------------------------------------------------------------------------
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("StartDebug")) {
                    // 保存, ロード処理を行い, シーンを再読み込み
                    SceneManager::getInstance()->changeScene(SceneManager::getInstance()->getCurrentScene()->GetName());
                    isActive_ = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ///=================================================================================================
        // Editor
        ///=================================================================================================

        // Camera
        CameraManager::getInstance()->DebugUpdate();

        ///-------------------------------------------------------------------------------------------------
        // Editors Update
        ///-------------------------------------------------------------------------------------------------
        for (auto& [name, editor] : editors_) {
            if (editorActive_[editor.get()]) {
                editor->Update();
            }
        }

        // Undo Redo
        Input* input = Input::getInstance();
        if (input->isPressKey(DIK_LCONTROL)) {
            if (input->isPressKey(DIK_LSHIFT)) {
                // SHIFT あり
                if (input->isTriggerKey(DIK_Z)) {
                    Redo();
                }
            } else {
                // SHIFT なし
                if (input->isTriggerKey(DIK_Z)) {
                    Undo();
                }
                if (input->isTriggerKey(DIK_Y)) {
                    Redo();
                }
            }
        }

    } else { // Debug のとき
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("EndDebug")) {
                    // ロード処理を行い, シーンを再読み込み
                    SceneManager::getInstance()->getCurrentScene()->Finalize(false);
                    SceneManager::getInstance()->getCurrentScene()->Initialize();

                    // Editor を再初期化
                    this->Initialize();
                    isActive_ = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}

void EngineEditor::Finalize() {
    for (auto& [editorName, editor] : editors_) {
        editor->Finalize();
    }
}
#endif // _DEBUG
