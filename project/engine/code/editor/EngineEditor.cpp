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

void EngineEditor::ExecuteCommandRequests() {
    while (true) {
        // request がなくなったら終了
        if (commandRequestQueue_.empty()) {
            break;
        }

        // command を取り出す
        auto command = std::move(commandRequestQueue_.front());
        commandRequestQueue_.pop();
        if (command == nullptr) {
            continue;
        }

        // command を実行
        command->Execute();

        // coomand を history に追加
        commandHistory_.erase(currentCommandItr_, commandHistory_.end());
        commandHistory_.push_back(std::move(command));
        currentCommandItr_ = commandHistory_.end();
    }
}

EngineEditor* EngineEditor::getInstance() {
    static EngineEditor instance;
    return &instance;
}

void EngineEditor::Initialize() {
    ///============================= Editor の初期化 ========================================
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
                if (ImGui::BeginMenu("Change Startup Scene")) {
                    SceneManager* sceneManager = SceneManager::getInstance();
                    SerializedField<std::string> startupSceneName("Settings", "Scene", "StartupSceneName");
                    ImGui::Text("Startup Scene Name: %s", startupSceneName->c_str());
                    for (auto& [name, index] : sceneManager->sceneIndexs_) {
                        if (ImGui::MenuItem(name.c_str())) {
                            startupSceneName.setValue(name);
                            GlobalVariables::getInstance()->SaveFile("Settings", "Scene");
                            break;
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Change EditScene")) {
                    SceneManager* sceneManager = SceneManager::getInstance();
                    for (auto& [name, index] : sceneManager->sceneIndexs_) {
                        if (ImGui::MenuItem(name.c_str())) {
                            sceneManager->changeScene(name);
                            // Editor を再初期化
                            this->Initialize();
                            break;
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Save")) {
                    SceneManager::getInstance()->getCurrentScene()->SaveSceneEntity();
                }
                if (ImGui::MenuItem("Reload")) {
                    SceneManager::getInstance()->getCurrentScene()->Finalize();
                    SceneManager::getInstance()->getCurrentScene()->Initialize();

                    // Editor を再初期化
                    this->Initialize();
                }

                ImGui::EndMenu();
            }

            /// ------------------------------------------------------------------------------------------------
            // Editors
            /// ------------------------------------------------------------------------------------------------
            if (ImGui::BeginMenu("Editors")) {
                if (ImGui::BeginMenu("ActiveState")) {
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
                if (ImGui::BeginMenu("StartDebug")) {
                    if (ImGui::MenuItem("Startup Scene")) {
                        // 保存, ロード処理を行い, シーンを再読み込み
                        SceneManager::getInstance()->changeScene(SceneManager::getInstance()->getCurrentScene()->GetName());
                        isActive_ = false;
                    }
                    if (ImGui::MenuItem("Current Scene")) {
                        const auto& scene = SceneManager::getInstance()->getCurrentScene();
                        scene->Finalize(false);
                        scene->Initialize();

                        isActive_ = false;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ///=================================================================================================
        // Main DockSpace Window
        ///=================================================================================================
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport       = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("MainEditorWindow", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        // DockSpaceを作成
        ImGuiID dockspace_id = ImGui::GetID("MainEditorWindow");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ///=================================================================================================
        // Scene
        ///=================================================================================================
        SceneManager::getInstance()->DebugUpdate();

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

        ///-------------------------------------------------------------------------------------------------
        // Commandの実行
        ///-------------------------------------------------------------------------------------------------
        ExecuteCommandRequests();

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

        ImGui::End();
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
