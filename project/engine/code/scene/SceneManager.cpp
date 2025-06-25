#include "SceneManager.h"

// Interface
#include "scene/Scene.h"

/// stl
#include <format>

/// engine
#include "Engine.h"
#include "winApp/WinApp.h"

#define ENGINE_INPUT
#define RESOURCE_DIRECTORY
#define DELTA_TIME
#include "EngineInclude.h"
// ecs
#include "component/ComponentArray.h"
#include "ECS/Entity.h"
#include "system/ISystem.h"

// directX12Object
#include "directX12/RenderTexture.h"
// module
#include "camera/CameraManager.h"
#include "editor/EditorController.h"
#include "module/debugger/DebuggerGroup.h"
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
    // シーンの初期化
    std::string startSceneName = startupSceneName_;
    currentScene_              = std::make_unique<Scene>(startSceneName);
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->getSceneView()->Resize(Engine::getInstance()->getWinApp()->getWindowSize());

    Engine::getInstance()->addWindowResizeEvent(
        [this](const Vec2f& newSize) {
            if (currentScene_) {
                currentScene_->getSceneView()->Resize(newSize);
            }
        });
    // デバッグカメラの初期化
}

void SceneManager::Finalize() {}

void SceneManager::Update() {
    if (isChangeScene_) {
        // SceneChange
        executeSceneChange();
    } else {
        currentScene_->Update();
    }

    currentScene_->Render();

    Engine::getInstance()->ScreenPreDraw();
    currentScene_->getSceneView()->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();
}

// #ifdef _DEBUG
// #include <imgui/imgui.h>
// #include <imgui/ImGuizmo/ImGuizmo.h>
//
// void SceneManager::DebugUpdate() {
//     static ImVec2 s_buttonIconSize(16, 16);
//
//     ///=================================================================================================
//     // Main DockSpace Window
//     ///=================================================================================================
//     ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
//     ImGuiViewport* viewport       = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(viewport->Pos);
//     ImGui::SetNextWindowSize(viewport->Size);
//     ImGui::SetNextWindowViewport(viewport->ID);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//     window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//     window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//
//     ImGui::Begin("MainEditorWindow", nullptr, window_flags);
//     ImGui::PopStyleVar(2);
//
//     // DockSpaceを作成
//     ImGuiID dockspace_id = ImGui::GetID("MainEditorWindow");
//     ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
//
//     ImGui::End();
//
//     ///=================================================================================================
//     // SceneView
//     ///=================================================================================================
//     if (ImGui::Begin("SceneView")) {
//         ImVec2 sceneViewPos  = ImGui::GetCursorScreenPos(); // SceneView の左上のスクリーン座標
//         ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
//
//         // SceneView の元の解像度
//         Vec2f originalResolution = sceneView_->getTextureSize();
//
//         // アスペクト比を維持しながらサイズを調整
//         float aspectRatio = originalResolution[X] / originalResolution[Y];
//         if (sceneViewSize.x / sceneViewSize.y > aspectRatio) {
//             sceneViewSize.x = sceneViewSize.y * aspectRatio;
//         } else {
//             sceneViewSize.y = sceneViewSize.x / aspectRatio;
//         }
//
//         // SceneView を描画
//         ImGui::Image(reinterpret_cast<ImTextureID>(sceneView_->getBackBufferSrvHandle().ptr), sceneViewSize);
//
//         // マウス座標を取得
//         Vec2f mousePos = Input::getInstance()->getCurrentMousePos();
//
//         // マウス座標をゲーム内の座標に変換
//         Vec2f gamePos = ConvertMouseToSceneView(mousePos, sceneViewPos, sceneViewSize, originalResolution);
//         Input::getInstance()->setVirtualMousePos(gamePos);
//
//         // ImGuizmo のフレーム開始
//         ImGuizmo::BeginFrame();
//
//         // ImGuizmo の設定
//         ImGuizmo::SetOrthographic(false); // 透視投影かどうか
//         ImGuizmo::SetDrawlist();
//
//         // ImGuizmo のウィンドウサイズ・位置を設定
//         ImGuizmo::SetRect(sceneViewPos.x, sceneViewPos.y, sceneViewSize.x, sceneViewSize.y);
//
//         //! TODO : デカップリング
//         ///=================================================================================================
//         // Editor / DebuggerGroup
//         ///=================================================================================================
//         switch (currentSceneState_) {
//         case SceneManager::SceneState::Edit: {
//             ///=================================================================================================
//             // MainMenuBar
//             ///=================================================================================================
//             auto SaveScene = [&]() {
//                 SceneSerializer serializer;
//                 serializer.Serialize(currentSceneName_);
//             };
//             if (ImGui::BeginMainMenuBar()) {
//                 /// ------------------------
//                 // Scene
//                 /// ------------------------
//                 if (ImGui::BeginMenu("Scene")) {
//                     if (ImGui::BeginMenu("Change Startup Scene")) {
//                         ImGui::Text("Startup Scene Name: %s", startupSceneName_->c_str());
//
//                         for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
//                             if (ImGui::MenuItem(name.c_str())) {
//                                 startupSceneName_.setValue(name);
//                                 GlobalVariables::getInstance()->SaveFile("Settings", "Scene");
//                                 break;
//                             }
//                         }
//                         ImGui::EndMenu();
//                     }
//                     if (ImGui::BeginMenu("Change EditScene")) {
//                         SceneManager* sceneManager = SceneManager::getInstance();
//                         for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
//                             if (ImGui::MenuItem(name.c_str())) {
//                                 sceneManager->changeScene(name);
//
//                                 // Editor を再初期化
//                                 editorController_->Finalize();
//                                 editorController_->Initialize();
//                                 break;
//                             }
//                         }
//                         ImGui::EndMenu();
//                     }
//                     if (ImGui::BeginMenu("Create NewScene")) {
//                         // シーンの新規作成
//                         ImGui::InputText("SceneName", newSceneName_, sizeof(newSceneName_));
//                         if (ImGui::Button("Create")) {
//                             // 重複 チェック
//                             for (auto& [directory, name] : myfs::searchFile(kApplicationResourceDirectory + "/scene", "json")) {
//                                 if (name == newSceneName_) {
//                                     std::string message = std::format("{} already exists", newSceneName_);
//                                     MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);
//
//                                     ImGui::EndMenu();
//                                     ImGui::EndMenu();
//                                     ImGui::EndMainMenuBar();
//                                     return;
//                                 }
//                             }
//
//                             std::string message = std::format("{} save it?", newSceneName_);
//                             if (MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OKCANCEL) == IDOK) {
//                                 SceneFinalize();
//
//                                 // シーンの新規作成
//                                 // 最初はすべてのシステムをオンにする
//                                 ecsManager_->AllActivateSystem();
//
//                                 // 保存
//                                 SceneSerializer serializer;
//                                 serializer.SerializeFromJson(newSceneName_);
//                                 message = std::format("{} saved", newSceneName_);
//
//                                 MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);
//                             }
//                         }
//                         ImGui::EndMenu();
//                     }
//                     if (ImGui::MenuItem("Save")) {
//                         // シーンの保存
//                         SaveScene();
//                         EditorController::getInstance()->clearCommandHistory();
//                     }
//                     if (ImGui::MenuItem("Reload")) {
//                         this->changeScene(currentSceneName_);
//
//                         editorController_->Finalize();
//                         editorController_->Initialize();
//                     }
//
//                     ImGui::EndMenu();
//                 }
//
//                 /// ------------------------
//                 // Editors
//                 /// ------------------------
//                 if (ImGui::BeginMenu("Editors")) {
//                     if (ImGui::BeginMenu("ActiveState")) {
//                         for (auto& [name, editor] : editorController_->editors_) {
//                             ImGui::Checkbox(name.c_str(), editorController_->editorActivity_[editor.get()]);
//                         }
//                         ImGui::EndMenu();
//                     }
//                     ImGui::EndMenu();
//                 }
//
//                 /// ------------------------
//                 // Debug
//                 /// ------------------------
//                 if (ImGui::BeginMenu("Debug")) {
//                     if (ImGui::BeginMenu("StartDebug")) {
//                         if (ImGui::MenuItem("Startup Scene")) {
//                             // 保存, ロード処理を行い, シーンを再読み込み
//                             SceneManager::getInstance()->changeScene(startupSceneName_);
//                             currentSceneState_ = SceneState::Debug;
//                         }
//                         if (ImGui::MenuItem("Current Scene")) {
//                             SceneManager::getInstance()->changeScene(currentSceneName_);
//
//                             currentSceneState_ = SceneState::Debug;
//                         }
//                         ImGui::EndMenu();
//                     }
//                     ImGui::EndMenu();
//                 }
//                 ImGui::EndMainMenuBar();
//             }
//
//             Input* input = Input::getInstance();
//             if (input->isPressKey(Key::L_CTRL) && input->isTriggerKey(Key::S)) {
//                 // Ctrl + S でシーンを保存
//                 SaveScene();
//             }
//
//             if (ImGui::Begin("Debugger")) {
//                 if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(playIcon_).ptr), s_buttonIconSize)) {
//                     // play
//                     currentSceneState_ = SceneState::Debug;
//
//                     SceneManager::getInstance()->changeScene(currentSceneName_);
//                 }
//                 ImGui::SameLine();
//                 ImGui::Text("DeltaTime :%.4f", Engine::getInstance()->getDeltaTime());
//             }
//             ImGui::End();
//
//             if (currentSceneState_ == SceneState::Debug) {
//                 // シーンを保存
//                 SceneSerializer serializer;
//                 serializer.Serialize(currentSceneName_);
//
//                 // Editor を終了
//                 editorController_->Finalize();
//                 // DebuggerGroup を再初期化
//                 debuggerGroup_->Initialize();
//
//                 debugState_ = DebugState::Play;
//                 break;
//             }
//
//             ///=================================================================================================
//             // EditorGroup
//             ///=================================================================================================
//             editorController_->Update();
//
//             CameraManager::getInstance()->DebugUpdate();
//
//             break;
//         }
//         case SceneManager::SceneState::Debug: {
//             if (ImGui::BeginMainMenuBar()) {
//                 if (ImGui::BeginMenu("Debug")) {
//                     if (ImGui::BeginMenu("DebuggerGroup")) {
//                         for (auto& [name, debugger] : debuggerGroup_->debuggersActive_) {
//                             ImGui::Checkbox(name.c_str(), &debuggerGroup_->debuggersActive_[name]);
//                         }
//                         ImGui::EndMenu();
//                     }
//                     if (ImGui::MenuItem("EndDebug")) {
//                         currentSceneState_ = SceneState::Edit;
//                     }
//                     ImGui::EndMenu();
//                 }
//                 ImGui::EndMainMenuBar();
//             }
//
//             if (ImGui::Begin("Debugger")) {
//                 if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(stopIcon_).ptr), s_buttonIconSize)) {
//                     // Stop
//                     currentSceneState_ = SceneState::Edit;
//                     debugState_        = DebugState::Stop;
//                 }
//                 ImGui::SameLine();
//                 if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(rePlayIcon_).ptr), s_buttonIconSize)) {
//                     // RePlay
//                     currentSceneState_ = SceneState::Debug;
//                     debugState_        = DebugState::RePlay;
//                 }
//                 ImGui::SameLine();
//
//                 if (debugState_ == DebugState::Play) {
//                     if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(pauseIcon_).ptr), s_buttonIconSize)) {
//                         // Pause
//                         debugState_ = DebugState::Pause;
//                     }
//                 } else {
//                     // Pause のときは、PauseCircleIcon を表示
//                     if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(pauseCircleIcon_).ptr), s_buttonIconSize)) {
//                         // Pause
//                         debugState_ = DebugState::Play;
//                     }
//                     ImGui::SameLine();
//                     ImGui::Text("Pause Now");
//                 }
//
//                 if (ImGui::ImageButton(
//                         reinterpret_cast<ImTextureID>(TextureManager::getDescriptorGpuHandle(cameraIcon_).ptr),
//                         s_buttonIconSize)) {
//                     // DebugCamera
//                     isUsingDebugCamera_ = !isUsingDebugCamera_;
//                 }
//
//                 if (isUsingDebugCamera_) {
//                     ImGui::SameLine();
//                     ImGui::Text("DebugCamera: On");
//                     CameraManager::getInstance()->DebugUpdate();
//                 }
//
//                 ImGui::Text("DeltaTime :%.4f", Engine::getInstance()->getDeltaTime());
//             }
//             ImGui::End();
//
//             if (currentSceneState_ == SceneState::Edit) {
//                 // Editor を再初期化
//                 editorController_->Initialize();
//
//                 // 保存しない
//                 SceneFinalize();
//                 SceneInitialize(currentSceneName_);
//                 break;
//             }
//             if (debugState_ == DebugState::RePlay) {
//                 // シーンを再初期化
//                 SceneFinalize();
//                 SceneInitialize(currentSceneName_);
//
//                 // DebuggerGroup を終了
//                 debuggerGroup_->Finalize();
//                 // DebuggerGroup を再初期化
//                 debuggerGroup_->Initialize();
//
//                 debugState_ = DebugState::Play;
//             }
//
//             ///=================================================================================================
//             // DebuggerGroup
//             ///=================================================================================================
//             debuggerGroup_->Update();
//
//             break;
//         }
//         default:
//             break;
//         }
//     }
//     ImGui::End();
// }
// #endif

const std::string& SceneManager::getCurrentSceneName() const { return currentScene_->getName(); }

void SceneManager::sceneChange2StartupScene() {
    // シーンの初期化
    currentScene_ = std::make_unique<Scene>(startupSceneName_);
    SceneSerializer serializer(currentScene_.get());
}

void SceneManager::changeScene(const std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}

void SceneManager::executeSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentScene_->getName(), changingSceneName_);

    currentScene_->Finalize();
    currentScene_ = std::make_unique<Scene>(changingSceneName_);

    {
        SceneSerializer serializer(currentScene_.get());
        serializer.Deserialize();
    }

    currentScene_->Initialize();

    isChangeScene_ = false;
}

const std::string SceneSerializer::directory_ = kApplicationResourceDirectory + "/scene/";

SceneSerializer::SceneSerializer(Scene* _targetScene) : targetScene_(_targetScene) {}

SceneSerializer::~SceneSerializer() {}

bool SceneSerializer::Serialize() {
    if (!targetScene_) {
        LOG_ERROR("Target scene is null");
        return false;
    }
    std::string message = std::format("{} save it?", targetScene_->getName());

    if (MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OKCANCEL) != IDOK) {
        return false;
    }

    // 保存
    SerializeFromJson();

    message = std::format("{} saved", targetScene_->getName());
    MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OK);
    return true;
}

void SceneSerializer::Deserialize() {
    DeserializeFromJson();
}

void SceneSerializer::SerializeFromJson() {
    nlohmann::json jsonData = nlohmann::json::object();

    /// =====================================================
    // Entity
    /// =====================================================
    auto& entities = targetScene_->entityRepository_;

    std::list<GameEntity*> aliveEntities;
    for (auto& entity : entities->getEntitiesRef()) {
        if (entity.isAlive()) {
            aliveEntities.push_back(&entity);
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
        {
            nlohmann::json systemsData = nlohmann::json::array();
            const auto& systems        = targetScene_->systemRunner_->getSystems();
            for (const auto& systemByCategory : systems) {
                for (const auto& [name, system] : systemByCategory) {
                    if (system->hasEntity(entity)) {
                        systemsData.push_back(
                            {{"SystemCategory", system->getCategory()},
                                {"SystemName", name}});
                    }
                }
            }
            entityData["Systems"] = systemsData;
        }

        // コンポーネントを保存
        const auto& componentRepository = targetScene_->componentRepository_;
        nlohmann::json componentsData;
        for (const auto& [componentTypeName, componentArray] : componentRepository->getComponentArrayMap()) {
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
    {
        const auto& systems        = targetScene_->systemRunner_->getSystems();
        nlohmann::json systemsData = nlohmann::json::array();

        for (const auto& systemByCategory : systems) {
            nlohmann::json systemDataByCategory = nlohmann::json::array();
            for (const auto& [name, system] : systemByCategory) {
                if (!system || !system->isActive()) {
                    continue; // 無効なシステムはスキップ
                }
                nlohmann::json systemData = nlohmann::json::object();
                systemData["SystemName"]  = name;
                systemData["Priority"]    = system->getPriority();
                systemDataByCategory.push_back(systemData);
            }
            systemsData.push_back(systemDataByCategory);
        }

        jsonData["Systems"] = systemsData;
    }

    // JSON ファイルに書き込み
    myfs::createFolder(directory_);
    std::ofstream ofs(directory_ + targetScene_->getName() + ".json");
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", targetScene_->getName());
        return;
    }
    ofs << std::setw(4) << jsonData << std::endl;
    ofs.close();
}

void SceneSerializer::DeserializeFromJson() {
    std::ifstream ifs(directory_ + targetScene_->getName() + ".json");
    if (!ifs) {
        LOG_ERROR("Failed to open JSON file for reading: {}", targetScene_->getName());
        return;
    }

    nlohmann::json jsonData;
    ifs >> jsonData;
    ifs.close();

    auto& entityRepository    = targetScene_->entityRepository_;
    auto& componentRepository = targetScene_->componentRepository_;
    auto& systemRunner        = targetScene_->systemRunner_;

    /// =====================================================
    // Entity
    /// =====================================================
    for (auto& entityData : jsonData["Entities"]) {
        std::string entityName = entityData["Name"];
        bool isUnique          = entityData["isUnique"];

        int32_t entityID   = entityRepository->registerEntity(entityName, isUnique);
        GameEntity* entity = entityRepository->getEntity(entityID);

        // 所属するシステムを読み込み
        auto& sceneSystems = systemRunner->getSystemsRef();
        for (auto& systemData : entityData["Systems"]) {
            int32_t systemCategory = systemData["SystemType"];
            std::string systemName = systemData["SystemName"];
            ISystem* system        = sceneSystems[systemCategory][systemName];
            if (system) {
                system->addEntity(entity);
            } else {
                LOG_WARN("System not found: {} in category {}", systemName, systemCategory);
            }
        }

        // コンポーネントを読み込み
        const auto& componentArrayMap = componentRepository->getComponentArrayMap();
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
    int32_t systemCategoryIndex = 0;
    nlohmann::json& systems     = jsonData["Systems"];
    auto& sceneSystems          = systemRunner->getSystemsRef();
    for (auto& systemByType : systems) {
        for (auto& [systemName, system] : systemByType.items()) {
            ISystem* systemPtr = sceneSystems[systemCategoryIndex][systemName];
            if (systemPtr) {
                systemPtr->setPriority(system["Priority"]);
                systemRunner->registerSystem(systemName, true);
            }
        }
        ++systemCategoryIndex;
    }
}

void SceneSerializer::SaveEntity(int32_t _entityID, const std::string& _directory) {
    GameEntity* _entity = targetScene_->entityRepository_->getEntity(_entityID);

    if (!_entity || !_entity->isAlive()) {
        return;
    }

    nlohmann::json entityData;
    EntityToJson(_entityID, entityData);

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

void SceneSerializer::EntityToJson(int32_t _entityID, nlohmann::json& entityData) {
    GameEntity* _entity = targetScene_->entityRepository_->getEntity(_entityID);
    if (!_entity || !_entity->isAlive()) {
        return;
    }

    entityData["Name"]     = _entity->getDataType();
    entityData["isUnique"] = _entity->isUnique();

    // 所属するシステムを保存
    entityData["Systems"]    = nlohmann::json::array();
    auto& systemsJson        = entityData["Systems"];
    const auto& sceneSystems = targetScene_->systemRunner_->getSystems();
    for (const auto& systemsByType : sceneSystems) {
        for (const auto& [systemName, system] : systemsByType) {
            if (system->hasEntity(_entity)) {
                systemsJson.push_back({{"SystemCategory", system->getCategory()}, {"SystemName", systemName}});
            }
        }
    }

    // コンポーネントを保存
    const auto& componentArrayMap = targetScene_->componentRepository_->getComponentArrayMap();
    nlohmann::json componentsData;
    for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
        if (componentArray->hasEntity(_entity)) {
            componentArray->SaveComponent(_entity, componentsData);
        }
    }
    entityData["Components"] = componentsData;
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

    std::string entityName = entityData["Name"];
    bool isUnique          = entityData["isUnique"];
    int32_t entityID       = targetScene_->entityRepository_->registerEntity(entityName, isUnique);
    GameEntity* entity     = targetScene_->entityRepository_->getEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->getSystemsRef();
    for (auto& systemData : entityData["Systems"]) {
        int32_t systemCategory = systemData["SystemCategory"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = sceneSystems[systemCategory][systemName];
        if (system) {
            system->addEntity(entity);
        }
    }

    // コンポーネントを読み込み
    auto& componentArrayMap = targetScene_->componentRepository_->getComponentArrayMapRef();
    for (auto& [componentTypename, componentData] : entityData["Components"].items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }

    return entity;
}

GameEntity* SceneSerializer::EntityFromJson(const nlohmann::json& _entityData) {
    std::string entityName = _entityData.at("Name");
    bool isUnique          = _entityData.at("isUnique");
    int32_t entityID       = targetScene_->entityRepository_->registerEntity(entityName, isUnique);
    GameEntity* entity     = targetScene_->entityRepository_->getEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->getSystemsRef();
    for (auto& systemData : _entityData.at("Systems")) {
        int32_t systemCategory = systemData.at("SystemCategory");
        std::string systemName = systemData.at("SystemName");
        ISystem* system        = sceneSystems[systemCategory][systemName];
        if (system) {
            system->addEntity(entity);
        }
    }
    // コンポーネントを読み込み
    auto& componentArrayMap = targetScene_->componentRepository_->getComponentArrayMapRef();
    for (auto& [componentTypename, componentData] : _entityData.at("Components").items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }
    return entity;
}

GameEntity* SceneSerializer::EntityFromJson(int32_t _entityId, const nlohmann::json& _entityData) {
    std::string entityName = _entityData["Name"];
    bool isUnique          = _entityData["isUnique"];
    int32_t entityID       = targetScene_->entityRepository_->registerEntity(_entityId, entityName, isUnique);
    if (entityID < 0) {
        LOG_ERROR("Failed to register entity with ID: {}", _entityId);
        return nullptr;
    }
    GameEntity* entity = targetScene_->entityRepository_->getEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->getSystemsRef();
    for (auto& systemData : _entityData["Systems"]) {
        int32_t systemCategory = systemData["SystemCategory"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = sceneSystems[systemCategory][systemName];
        if (system) {
            system->addEntity(entity);
        }
    }
    // コンポーネントを読み込み
    auto& componentArrayMap = targetScene_->componentRepository_->getComponentArrayMapRef();
    for (auto& [componentTypename, componentData] : _entityData["Components"].items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }
    return entity;
}

GameEntity* getSceneEntity(int32_t _entityID) {
    Scene* currentScene = SceneManager::getInstance()->getCurrentScene();

    return currentScene->getEntityRepositoryRef()->getEntity(_entityID);
}

GameEntity* getSceneUniqueEntity(const std::string& _name) {
    Scene* currentScene = SceneManager::getInstance()->getCurrentScene();

    return currentScene->getEntityRepositoryRef()->getUniqueEntity(_name);
}
