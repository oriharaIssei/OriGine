#include "SceneManager.h"

// Interface
#include "iScene/IScene.h"

/// engine
#include "ECS/ECSManager.h"
#include "Engine.h"
// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/RenderTexture.h"
// module
#include "camera/CameraManager.h"
#include "module/debugger/DebuggerGroup.h"
#include "module/editor/EditorGroup.h"

/// math
#include "math/Vector2.h"
#include "math/Vector4.h"

SceneManager* SceneManager::getInstance() {
    static SceneManager instance;
    return &instance;
}

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

void SceneManager::Initialize() {
    sceneViewRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
    sceneViewSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

    ecsManager_ = EntityComponentSystemManager::getInstance();
    ecsManager_->Initialize();

    sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand(), sceneViewRtvArray_.get(), sceneViewSrvArray_.get());
    /// TODO
    // fix MagicNumber
    sceneView_->Initialize({1280.0f, 720.0f}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.0f, 0.0f, 0.0f, 1.0f});

#ifdef _DEBUG
    editorGroup_   = EditorGroup::getInstance();
    debuggerGroup_ = DebuggerGroup::getInstance();
#endif // _DEBUG
}

void SceneManager::Finalize() {
    ecsManager_->Finalize();

    // EditorModeのときだけ 保存する
    currentScene_->Finalize(inEditMode());
    scenes_.clear();

    sceneView_->Finalize();

    sceneViewRtvArray_->Finalize();
    sceneViewSrvArray_->Finalize();
}

void SceneManager::Update() {
    auto SceneChange = [this]() {
        if (currentScene_) {
            currentScene_->Finalize();
        }

        currentScene_ = scenes_[sceneIndexs_[changingSceneName_]]();
        currentScene_->Initialize();

        isChangeScene_ = false;
    };

    if (isChangeScene_) {
        // SceneChange
        SceneChange();
    }

    ecsManager_->Run();

    CameraManager::getInstance()->DataConvertToBuffer();
}

void SceneManager::Draw() {
    Engine::getInstance()->ScreenPreDraw();
    sceneView_->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();
}

#ifdef _DEBUG
#include "imgui/imgui.h"
void SceneManager::DebugUpdate() {
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

    ImGui::End();

    ///=================================================================================================
    // SceneView
    ///=================================================================================================
    if (ImGui::Begin("SceneView")) {
        ImGui::LabelText("SceneName", currentScene_->GetName().c_str());
        ImGui::Image(reinterpret_cast<ImTextureID>(sceneView_->getSrvHandle().ptr), ImGui::GetWindowSize());
        if (ImGui::IsItemFocused()) {
            // Camera の操作 (SceneView が focusされているときだけ)
            CameraManager::getInstance()->DebugUpdate();
        }
    }
    ImGui::End();

    ///=================================================================================================
    // Editor / DebuggerGroup
    ///=================================================================================================
    switch (currentSceneState_) {
    case SceneManager::SceneState::Edit:
        ///=================================================================================================
        // MainMenuBar
        ///=================================================================================================
        if (ImGui::BeginMainMenuBar()) {
            /// ------------------------
            // Scene
            /// ------------------------
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
                    currentScene_->SaveSceneEntity();
                }
                if (ImGui::MenuItem("Reload")) {
                    currentScene_->Finalize();
                    currentScene_->Initialize();

                    // Editor を再初期化
                    this->Initialize();
                }

                ImGui::EndMenu();
            }

            /// ------------------------
            // Editors
            /// ------------------------
            if (ImGui::BeginMenu("Editors")) {
                if (ImGui::BeginMenu("ActiveState")) {
                    for (auto& [name, editor] : editorGroup_->editors_) {
                        ImGui::Checkbox(name.c_str(), &editorGroup_->editorActive_[editor.get()]);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            /// ------------------------
            // Debug
            /// ------------------------
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::BeginMenu("StartDebug")) {
                    if (ImGui::MenuItem("Startup Scene")) {
                        // 保存, ロード処理を行い, シーンを再読み込み
                        SceneManager::getInstance()->changeScene(currentScene_->GetName());
                        currentSceneState_ = SceneState::Debug;
                    }
                    if (ImGui::MenuItem("Current Scene")) {
                        const auto& scene = currentScene_;
                        scene->Finalize();
                        scene->Initialize();

                        currentSceneState_ = SceneState::Debug;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (currentSceneState_ == SceneState::Debug) {
            // DebuggerGroup を終了
            debuggerGroup_->Finalize();
            // Editor を再初期化
            editorGroup_->Initialize();
            break;
        }

        ///=================================================================================================
        // EditorGroup
        ///=================================================================================================
        editorGroup_->Update();

        break;
    case SceneManager::SceneState::Debug:
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::BeginMenu("DebuggerGroup")) {
                    for (auto& [name, debugger] : debuggerGroup_->debuggersActive_) {
                        ImGui::Checkbox(name.c_str(), &debuggerGroup_->debuggersActive_[name]);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("EndDebug")) {
                    // ロード処理を行い, シーンを再読み込み
                    currentScene_->Finalize(false);
                    currentScene_->Initialize();

                    currentSceneState_ = SceneState::Edit;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (currentSceneState_ == SceneState::Edit) {
            // Editor を再初期化
            editorGroup_->Initialize();

            // DebuggerGroup を終了
            debuggerGroup_->Finalize();
            break;
        }

        ///=================================================================================================
        // DebuggerGroup
        ///=================================================================================================
        debuggerGroup_->Update();

        break;
    default:
        break;
    }
}
#endif

void SceneManager::addScene(
    const std::string& name,
    std::function<std::unique_ptr<IScene>()> _sceneMakeFunc) {
    sceneIndexs_[name] = static_cast<int32_t>(scenes_.size());
    scenes_.push_back(std::move(_sceneMakeFunc));
}

void SceneManager::changeScene(const std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}
