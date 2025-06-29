#include "SceneManager.h"

// Interface
#include "iScene/IScene.h"

/// stl
#include <format>

/// engine
#include "ECS/ECSManager.h"
#include "Engine.h"

#define ENGINE_INPUT
#define RESOURCE_DIRECTORY
#define DELTA_TIME
#include "EngineInclude.h"

// directX12Object
#include "directX12/RenderTexture.h"
// module
#include "camera/CameraManager.h"
#include "module/debugger/DebuggerGroup.h"
#include "module/editor/EditorController.h"
#include "texture/TextureManager.h"

// lib
#include "logger/Logger.h"
#include "myFileSystem/MyFileSystem.h"

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
    ecsManager_ = EntityComponentSystemManager::getInstance();
    ecsManager_->Initialize();

    sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand());
    sceneView_->setTextureName("SceneViewTexture");
    /// TODO
    // fix MagicNumber
    sceneView_->Initialize(2, {1280.0f, 720.0f}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.0f, 0.0f, 0.0f, 1.0f});

#ifdef _DEBUG
    editorController_ = EditorController::getInstance();
    debuggerGroup_    = DebuggerGroup::getInstance();

    playIcon_        = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/play.png");
    rePlayIcon_      = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/rePlay.png");
    stopIcon_        = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/stop.png");
    pauseIcon_       = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/pause.png");
    pauseCircleIcon_ = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/pauseCir.png");
    cameraIcon_      = TextureManager::LoadTexture(kEngineResourceDirectory + "/Texture/camera.png");
#endif // _DEBUG
}

void SceneManager::Finalize() {
    sceneView_->Finalize();

    ecsManager_->Finalize();
}

void SceneManager::Update() {

    ecsManager_->Run();

    Engine::getInstance()->ScreenPreDraw();

#ifdef _DEBUG
    renderedSceneViewSrvHandle_ = sceneView_->getBackBufferSrvHandle();
#endif // _DEBUG
    sceneView_->DrawTexture();

    Engine::getInstance()->ScreenPostDraw();

    if (isChangeScene_) {
        // SceneChange
        executeSceneChange();
    }
}

#ifdef _DEBUG
#include <imgui/imgui.h>
#include <imgui/ImGuizmo/ImGuizmo.h>

static Vec2f ConvertMouseToSceneView(const Vec2f& mousePos, const ImVec2& sceneViewPos, const ImVec2& sceneViewSize, const Vec2f& originalResolution) {
    // SceneView 内での相対的なマウス座標を計算
    float relativeX = mousePos[X] - sceneViewPos.x;
    float relativeY = mousePos[Y] - sceneViewPos.y;

    // SceneView のスケールを計算
    float scaleX = originalResolution[X] / sceneViewSize.x;
    float scaleY = originalResolution[Y] / sceneViewSize.y;

    // ゲーム内の座標に変換
    Vec2f gamePos;
    gamePos[X] = relativeX * scaleX;
    gamePos[Y] = relativeY * scaleY;

    return gamePos;
}

void SceneManager::DebugUpdate() {
    static ImVec2 s_buttonIconSize(16, 16);

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
        ImVec2 sceneViewPos  = ImGui::GetCursorScreenPos(); // SceneView の左上のスクリーン座標
        ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();

        // SceneView の元の解像度
        Vec2f originalResolution = sceneView_->getTextureSize();

        // アスペクト比を維持しながらサイズを調整
        float aspectRatio = originalResolution[X] / originalResolution[Y];
        if (sceneViewSize.x / sceneViewSize.y > aspectRatio) {
            sceneViewSize.x = sceneViewSize.y * aspectRatio;
        } else {
            sceneViewSize.y = sceneViewSize.x / aspectRatio;
        }

        // SceneView を描画
        ImGui::Image(reinterpret_cast<ImTextureID>(renderedSceneViewSrvHandle_.ptr), sceneViewSize);

        // マウス座標を取得
        Vec2f mousePos = Input::getInstance()->getCurrentMousePos();

        // マウス座標をゲーム内の座標に変換
        Vec2f gamePos = ConvertMouseToSceneView(mousePos, sceneViewPos, sceneViewSize, originalResolution);
        Input::getInstance()->setVirtualMousePos(gamePos);

        // ImGuizmo のフレーム開始
        ImGuizmo::BeginFrame();

        // ImGuizmo の設定
        ImGuizmo::SetOrthographic(false); // 透視投影かどうか
        ImGuizmo::SetDrawlist();

        // ImGuizmo のウィンドウサイズ・位置を設定
        ImGuizmo::SetRect(sceneViewPos.x, sceneViewPos.y, sceneViewSize.x, sceneViewSize.y);

        //! TODO : デカップリング
        ///=================================================================================================
        // Editor / DebuggerGroup
        ///=================================================================================================
        switch (currentSceneState_) {
        case SceneManager::SceneState::Edit: {
            ///=================================================================================================
            // MainMenuBar
            ///=================================================================================================
            auto SaveScene = [&]() {
                SceneSerializer serializer;
                serializer.Serialize(currentSceneName_);
            };
            if (ImGui::BeginMainMenuBar()) {
                /// ------------------------
                // Scene
                /// ------------------------
                if (ImGui::BeginMenu("Scene")) {
                    if (ImGui::BeginMenu("Change Startup Scene")) {
                        ImGui::Text("Startup Scene Name: %s", startupSceneName_->c_str());

                        for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
                            if (ImGui::MenuItem(name.c_str())) {
                                startupSceneName_.setValue(name);
                                GlobalVariables::getInstance()->SaveFile("Settings", "Scene");
                                break;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Change EditScene")) {
                        SceneManager* sceneManager = SceneManager::getInstance();
                        for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
                            if (ImGui::MenuItem(name.c_str())) {
                                sceneManager->changeScene(name);

                                // Editor を再初期化
                                editorController_->Finalize();
                                editorController_->Initialize();
                                break;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Create NewScene")) {
                        // シーンの新規作成
                        ImGui::InputText("SceneName", newSceneName_, sizeof(newSceneName_));
                        if (ImGui::Button("Create")) {
                            // 重複 チェック
                            for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
                                if (name == newSceneName_) {
                                    std::string message = std::format("{} already exists", newSceneName_);
                                    MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);

                                    ImGui::EndMenu();
                                    ImGui::EndMenu();
                                    ImGui::EndMainMenuBar();
                                    return;
                                }
                            }

                            std::string message = std::format("{} save it?", newSceneName_);
                            if (MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OKCANCEL) == IDOK) {
                                SceneFinalize();

                                // シーンの新規作成
                                // 最初はすべてのシステムをオンにする
                                ecsManager_->AllActivateSystem();

                                // 保存
                                SceneSerializer serializer;
                                serializer.SerializeFromJson(newSceneName_);
                                message = std::format("{} saved", newSceneName_);

                                MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);
                            }
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("Save")) {
                        // シーンの保存
                        SaveScene();
                        EditorController::getInstance()->clearCommandHistory();
                    }
                    if (ImGui::MenuItem("Reload")) {
                        this->changeScene(currentSceneName_);

                        editorController_->Finalize();
                        editorController_->Initialize();
                    }

                    ImGui::EndMenu();
                }

                /// ------------------------
                // Editors
                /// ------------------------
                if (ImGui::BeginMenu("Editors")) {
                    if (ImGui::BeginMenu("ActiveState")) {
                        for (auto& [name, editor] : editorController_->editors_) {
                            ImGui::Checkbox(name.c_str(), editorController_->editorActivity_[editor.get()]);
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
                            SceneManager::getInstance()->changeScene(startupSceneName_);
                            currentSceneState_ = SceneState::Debug;
                        }
                        if (ImGui::MenuItem("Current Scene")) {
                            SceneManager::getInstance()->changeScene(currentSceneName_);

                            currentSceneState_ = SceneState::Debug;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            Input* input = Input::getInstance();
            if (input->isPressKey(Key::L_CTRL) && input->isTriggerKey(Key::S)) {
                // Ctrl + S でシーンを保存
                SaveScene();
            }

            if (ImGui::Begin("Debugger")) {
                if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(playIcon_).ptr), s_buttonIconSize)) {
                    // play
                    currentSceneState_ = SceneState::Debug;

                    SceneManager::getInstance()->changeScene(currentSceneName_);
                }
                ImGui::SameLine();
                ImGui::Text("DeltaTime :%.4f", Engine::getInstance()->getDeltaTime());
            }
            ImGui::End();

            if (currentSceneState_ == SceneState::Debug) {
                // シーンを保存
                SceneSerializer serializer;
                serializer.Serialize(currentSceneName_);

                // Editor を終了
                editorController_->Finalize();
                // DebuggerGroup を再初期化
                debuggerGroup_->Initialize();

                debugState_ = DebugState::Play;
                break;
            }

            ///=================================================================================================
            // EditorGroup
            ///=================================================================================================
            editorController_->Update();

            CameraManager::getInstance()->DebugUpdate();

            break;
        }
        case SceneManager::SceneState::Debug: {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Debug")) {
                    if (ImGui::BeginMenu("DebuggerGroup")) {
                        for (auto& [name, debugger] : debuggerGroup_->debuggersActive_) {
                            ImGui::Checkbox(name.c_str(), &debuggerGroup_->debuggersActive_[name]);
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("EndDebug")) {
                        currentSceneState_ = SceneState::Edit;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            if (ImGui::Begin("Debugger")) {
                if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(stopIcon_).ptr), s_buttonIconSize)) {
                    // Stop
                    currentSceneState_ = SceneState::Edit;
                    debugState_        = DebugState::Stop;
                }
                ImGui::SameLine();
                if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(rePlayIcon_).ptr), s_buttonIconSize)) {
                    // RePlay
                    currentSceneState_ = SceneState::Debug;
                    debugState_        = DebugState::RePlay;
                }
                ImGui::SameLine();

                if (debugState_ == DebugState::Play) {
                    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(pauseIcon_).ptr), s_buttonIconSize)) {
                        // Pause
                        debugState_ = DebugState::Pause;
                    }
                } else {
                    // Pause のときは、PauseCircleIcon を表示
                    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(pauseCircleIcon_).ptr), s_buttonIconSize)) {
                        // Pause
                        debugState_ = DebugState::Play;
                    }
                    ImGui::SameLine();
                    ImGui::Text("Pause Now");
                }

                if (ImGui::ImageButton(
                        reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(cameraIcon_).ptr),
                        s_buttonIconSize)) {
                    // DebugCamera
                    isUsingDebugCamera_ = !isUsingDebugCamera_;
                }

                if (isUsingDebugCamera_) {
                    ImGui::SameLine();
                    ImGui::Text("DebugCamera: On");
                    CameraManager::getInstance()->DebugUpdate();
                }

                ImGui::Text("DeltaTime :%.4f", Engine::getInstance()->getDeltaTime());
            }
            ImGui::End();

            if (currentSceneState_ == SceneState::Edit) {
                // Editor を再初期化
                editorController_->Initialize();

                // 保存しない
                SceneFinalize();
                SceneInitialize(currentSceneName_);
                break;
            }
            if (debugState_ == DebugState::RePlay) {
                // シーンを再初期化
                SceneFinalize();
                SceneInitialize(currentSceneName_);

                // DebuggerGroup を終了
                debuggerGroup_->Finalize();
                // DebuggerGroup を再初期化
                debuggerGroup_->Initialize();

                debugState_ = DebugState::Play;
            }

            ///=================================================================================================
            // DebuggerGroup
            ///=================================================================================================
            debuggerGroup_->Update();

            break;
        }
        default:
            break;
        }
    }
    ImGui::End();
}
#endif

void SceneManager::sceneChange2StartupScene() {
    // シーンの初期化
    currentSceneName_ = startupSceneName_->c_str();
    SceneInitialize(currentSceneName_);
}

void SceneManager::changeScene(const std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}

void SceneManager::executeSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentSceneName_, changingSceneName_);

    currentSceneName_ = changingSceneName_;

    SceneFinalize();

    SceneInitialize(currentSceneName_);

    isChangeScene_ = false;
}

const std::string SceneSerializer::directory_ = kApplicationResourceDirectory + "/scene/";

bool SceneSerializer::Serialize(const std::string& _sceneName) {
    std::string message = std::format("{} save it?", _sceneName);

    if (MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OKCANCEL) != IDOK) {
        return false;
    }

    // 保存
    SerializeFromJson(_sceneName);

    message = std::format("{} saved", _sceneName);
    MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);
    return true;
}

void SceneSerializer::Deserialize(const std::string& _sceneName) {
    DeserializeFromJson(_sceneName);
}

void SceneSerializer::SerializeFromJson(const std::string& _sceneName) {
    ECSManager* ecsManager  = ECSManager::getInstance();
    nlohmann::json jsonData = nlohmann::json::object();

    /// =====================================================
    // Entity
    /// =====================================================
    auto& entities = ecsManager->getEntities();

    std::list<GameEntity*> aliveEntities;
    for (auto& entity : entities) {
        if (entity.isAlive()) {
            aliveEntities.push_back(ecsManager->getEntity(entity.getID()));
        }
    }

    for (auto& entity : aliveEntities) {
        if (!entity->isAlive()) {
            continue;
        }
        nlohmann::json entityData = nlohmann::json::object();
        entityData["Name"]        = entity->getDataType();
        entityData["isUnique"]    = entity->isUnique();

        // 所属するシステムを保存
        const auto& systems = ecsManager->getSystems();
        for (const auto& systemsByType : systems) {
            for (const auto& [systemName, system] : systemsByType) {
                if (system->hasEntity(entity)) {
                    entityData["Systems"].push_back({{"SystemType", system->getSystemType()}, {"SystemName", systemName}});
                }
            }
        }

        // コンポーネントを保存
        const auto& componentArrayMap = ecsManager->getComponentArrayMap();
        nlohmann::json componentsData;
        for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
            if (componentArray->hasEntity(entity)) {
                componentArray->SaveComponent(entity, componentsData);
            }
        }
        entityData["Components"] = componentsData;

        jsonData["Entities"].push_back(entityData);
    }

    /// =====================================================
    // System
    /// =====================================================
    const auto& systems = ecsManager->getSystems();
    for (const auto& systemsByType : systems) {
        nlohmann::json systemByType = nlohmann::json::object();
        for (const auto& [systemName, system] : systemsByType) {
            if (!system->isActive()) {
                continue;
            }
            nlohmann::json systemData = nlohmann::json::object();
            systemData["Priority"]    = system->getPriority();

            systemByType[systemName] = systemData;
        }
        jsonData["Systems"].push_back(systemByType);
    }

    // JSON ファイルに書き込み
    myfs::createFolder(directory_);
    std::ofstream ofs(directory_ + _sceneName + ".json");
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", _sceneName);
        return;
    }
    ofs << std::setw(4) << jsonData << std::endl;
    ofs.close();
}

void SceneSerializer::DeserializeFromJson(const std::string& _sceneName) {
    std::ifstream ifs(directory_ + _sceneName + ".json");
    if (!ifs) {
        LOG_ERROR("Failed to open JSON file for reading: {}", _sceneName);
        return;
    }

    nlohmann::json jsonData;
    ifs >> jsonData;
    ifs.close();

    /// =====================================================
    // Entity
    /// =====================================================
    ECSManager* ecsManager = ECSManager::getInstance();
    for (auto& entityData : jsonData["Entities"]) {
        std::string entityName = entityData["Name"];
        int32_t entityID       = ecsManager->registerEntity(entityName);
        GameEntity* entity     = ecsManager->getEntity(entityID);

        bool isUnique = entityData["isUnique"];
        if (isUnique) {
            ecsManager->registerUniqueEntity(entity);
        }

        // 所属するシステムを読み込み
        for (auto& systemData : entityData["Systems"]) {
            int32_t systemType     = systemData["SystemType"];
            std::string systemName = systemData["SystemName"];
            ISystem* system        = ecsManager->getSystem(SystemType(systemType), systemName);
            if (system) {
                system->addEntity(entity);
            }
        }

        // コンポーネントを読み込み
        auto& componentArrayMap = ecsManager->getComponentArrayMap();
        for (auto& [componentTypename, componentData] : entityData["Components"].items()) {
            auto itr = componentArrayMap.find(componentTypename);
            if (itr != componentArrayMap.end()) {
                itr->second->LoadComponent(entity, componentData);
            }
        }
    }

    /// =====================================================
    // System
    /// =====================================================
    int32_t systemTypeIndex = 0;
    nlohmann::json& systems = jsonData["Systems"];
    for (auto& systemByType : systems) {
        for (auto& [systemName, system] : systemByType.items()) {
            ISystem* systemPtr = ecsManager->getSystem(SystemType(systemTypeIndex), systemName);
            if (systemPtr) {
                systemPtr->setPriority(system["Priority"]);
                ECSManager::getInstance()->ActivateSystem(systemName, SystemType(systemTypeIndex));
            }
        }
        systemTypeIndex++;
    }
}

void SceneSerializer::SaveEntity(GameEntity* _entity, const std::string& _directory) {
    if (!_entity || !_entity->isAlive()) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();
    nlohmann::json entityData;

    entityData["Name"]     = _entity->getDataType();
    entityData["isUnique"] = _entity->isUnique();

    // 所属するシステムを保存
    const auto& systems = ecsManager->getSystems();
    for (const auto& systemsByType : systems) {
        for (const auto& [systemName, system] : systemsByType) {
            if (system->hasEntity(_entity)) {
                entityData["Systems"].push_back({{"SystemType", system->getSystemType()}, {"SystemName", systemName}});
            }
        }
    }

    // コンポーネントを保存
    const auto& componentArrayMap = ecsManager->getComponentArrayMap();
    nlohmann::json componentsData;
    for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
        if (componentArray->hasEntity(_entity)) {
            componentArray->SaveComponent(_entity, componentsData);
        }
    }
    entityData["Components"] = componentsData;

    // ディレクトリを作成
    myFs::createFolder(_directory);
    std::string filePath = _directory + "/" + _entity->getDataType() + ".ent";
    // JSONファイルに書き込み
    std::ofstream ofs(filePath);
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", filePath);
        return;
    }
    ofs << std::setw(4) << entityData << std::endl;
    ofs.close();
}

GameEntity* SceneSerializer::LoadEntity(const std::string& _directory, const std::string& _dataType) {
    std::string filePath = _directory + "/" + _dataType + ".ent";
    std::ifstream ifs(filePath);
    if (!ifs) {
        LOG_ERROR("Failed to open JSON file for reading: {}", filePath);
        return nullptr;
    }

    nlohmann::json entityData;
    ifs >> entityData;
    ifs.close();

    ECSManager* ecsManager = ECSManager::getInstance();
    std::string entityName = entityData["Name"];
    int32_t entityID       = ecsManager->registerEntity(entityName);
    GameEntity* entity     = ecsManager->getEntity(entityID);

    bool isUnique = entityData["isUnique"];
    if (isUnique) {
        ecsManager->registerUniqueEntity(entity);
    }

    // 所属するシステムを読み込み
    for (auto& systemData : entityData["Systems"]) {
        int32_t systemType     = systemData["SystemType"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = ecsManager->getSystem(SystemType(systemType), systemName);
        if (system) {
            system->addEntity(entity);
        }
    }

    // コンポーネントを読み込み
    auto& componentArrayMap = ecsManager->getComponentArrayMap();
    for (auto& [componentTypename, componentData] : entityData["Components"].items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }

    return entity;
}
