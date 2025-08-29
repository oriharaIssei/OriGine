#include "SceneManager.h"

// Interface
#include "scene/Scene.h"

/// stl
#include <format>

/// engine
#include "Engine.h"
#include "logger/Logger.h"
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
#include "texture/TextureManager.h"
// util
#include "myFileSystem/MyFileSystem.h"

/// math
#include "math/Vector2.h"
#include "math/Vector4.h"

#pragma region "SceneManager"

SceneManager* SceneManager::getInstance() {
    static SceneManager instance;
    return &instance;
}

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

void SceneManager::Initialize(const std::string& _startScene) {
    // シーンの初期化
    currentScene_ = std::make_unique<Scene>(_startScene);
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->getSceneView()->Resize(Engine::getInstance()->getWinApp()->getWindowSize());

    Engine::getInstance()->addWindowResizeEvent(
        [](const Vec2f& newSize) {
            auto currentScene = SceneManager::getInstance()->getCurrentScene();
            if (!currentScene) {
                LOG_ERROR("Current scene is null. Cannot resize SceneView.");
                return;
            }

            auto sceneView = currentScene->getSceneView();
            if (!sceneView) {
                LOG_ERROR("SceneView is null. Cannot resize SceneView. CurrentSceneName : {}", currentScene->getName());
                return;
            }
            sceneView->Resize(newSize);
        });

#ifdef _DEVELOP
    fileWatcher_ = std::make_unique<FileWatcher>(kApplicationResourceDirectory + "/scene/" + _startScene + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP
}
void SceneManager::Initialize() {
    this->Initialize(startupSceneName_);
}

void SceneManager::Finalize() {
#ifdef _DEVELOP
    fileWatcher_->Stop();
#endif // _DEVELOP
}

void SceneManager::Update() {

    currentScene_->Update();

    currentScene_->Render();

    Engine::getInstance()->ScreenPreDraw();
    currentScene_->getSceneView()->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();

#ifdef _DEVELOP
    if (fileWatcher_->isChanged()) {
        std::string currentSceneName = currentScene_->getName();
        currentScene_->Finalize();
        currentScene_ = std::make_unique<Scene>(currentSceneName);

        currentScene_->Initialize();
    }
#endif // _DEVELOP

    if (isChangeScene_) {
        // SceneChange
        executeSceneChange();
    }
}

const std::string& SceneManager::getCurrentSceneName() const { return currentScene_->getName(); }

void SceneManager::changeScene(const std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}

void SceneManager::executeSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentScene_->getName(), changingSceneName_);

    currentScene_->Finalize();
    currentScene_ = std::make_unique<Scene>(changingSceneName_);

    currentScene_->Initialize();

#ifdef _DEVELOP
    // 監視対象を変更
    fileWatcher_->Stop();
    fileWatcher_->setFilePath(kApplicationResourceDirectory + "/scene/" + changingSceneName_ + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP

    isChangeScene_ = false;
}
#pragma endregion

#pragma region "SceneSerializer"

const std::string SceneSerializer::SceneDirectory = kApplicationResourceDirectory + "/scene/";

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
        if (entity.isAlive() && entity.shouldSave()) {
            aliveEntities.push_back(&entity);
        }
    }

    for (auto& entity : aliveEntities) {
        if (!entity->isAlive()) {
            continue;
        }
        nlohmann::json entityData = nlohmann::json::object();

        EntityToJson(entity->getID(), entityData);

        jsonData["Entities"].push_back(entityData);
    }

    /// =====================================================
    // System
    /// =====================================================
    {
        const auto& systems                 = targetScene_->systemRunner_->getSystems();
        nlohmann::json systemsData          = nlohmann::json::array();
        nlohmann::json systemDataByCategory = nlohmann::json::object();

        for (const auto& [name, system] : systems) {
            if (!system || !system->isActive()) {
                continue; // 無効なシステムはスキップ
            }
            nlohmann::json systemData  = nlohmann::json::object();
            systemData["Priority"]     = system->getPriority();
            systemDataByCategory[name] = systemData;
        }
        systemsData.push_back(systemDataByCategory);

        jsonData["Systems"] = systemsData;

        nlohmann::json categoryActivity = nlohmann::json::array();
        for (size_t i = 0; i < static_cast<size_t>(SystemCategory::Count); ++i) {
            categoryActivity.push_back(targetScene_->systemRunner_->getCategoryActivityRef()[i]);
        }
        jsonData["CategoryActivity"] = categoryActivity;
    }

    // JSON ファイルに書き込み
    std::string sceneFilePath = SceneDirectory + targetScene_->getName() + ".json";
    myfs::deleteFile(sceneFilePath);
    myfs::createFolder(SceneDirectory);
    std::ofstream ofs(sceneFilePath);
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", targetScene_->getName());
        return;
    }
    ofs << std::setw(4) << jsonData << std::endl;
    ofs.close();
}

void SceneSerializer::DeserializeFromJson() {
    std::ifstream ifs(SceneDirectory + targetScene_->getName() + ".json");
    if (!ifs) {
        LOG_ERROR("Failed to open JSON file for reading: {}", targetScene_->getName());
        return;
    }

    nlohmann::json jsonData;
    ifs >> jsonData;
    ifs.close();

    auto& systemRunner        = targetScene_->systemRunner_;

    /// =====================================================
    // System
    /// =====================================================
    int32_t systemCategoryIndex = 0;
    nlohmann::json& systems     = jsonData["Systems"];
    for (auto& systemByType : systems) {
        for (auto& [systemName, system] : systemByType.items()) {
            systemRunner->registerSystem(systemName, system["Priority"], true);
        }
        ++systemCategoryIndex;
    }

    nlohmann::json& systemCategoryActivities = jsonData["CategoryActivity"];
    for (int32_t category = 0; category < static_cast<int32_t>(SystemCategory::Count); ++category) {
        if (category < systemCategoryActivities.size()) {
            bool isActive = systemCategoryActivities[category].get<bool>();
            systemRunner->setCategoryActivity(static_cast<SystemCategory>(category), isActive);
        } else {
            LOG_WARN("System category activity data missing for category index: {}", category);
        }
    }

    /// =====================================================
    // Entity
    /// =====================================================
    for (auto& entityData : jsonData["Entities"]) {
        EntityFromJson(entityData);
    }

    systemRunner->InitializeActiveSystems();
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
    for (const auto& [systemName, system] : sceneSystems) {
        if (!system) {
            LOG_WARN("System not found: {}", systemName);
            continue; // 無効なシステムはスキップ
        }
        if (system->hasEntity(_entity)) {
            systemsJson.push_back({{"SystemCategory", system->getCategory()}, {"SystemName", systemName}});
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

    return EntityFromJson(entityData);
}

GameEntity* SceneSerializer::EntityFromJson(const nlohmann::json& _entityData) {
    std::string entityName = _entityData.at("Name");
    bool isUnique          = _entityData.at("isUnique");
    int32_t entityID       = targetScene_->entityRepository_->CreateEntity(entityName, isUnique);
    GameEntity* entity     = targetScene_->entityRepository_->getEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->getSystemsRef();
    for (auto& systemData : _entityData.at("Systems")) {
        // int32_t systemCategory = systemData.at("SystemCategory");
        std::string systemName = systemData.at("SystemName");
        ISystem* system        = sceneSystems[systemName].get();
        if (system) {
            system->addEntity(entity);
        }
    }
    // コンポーネントを読み込み
    for (auto& [componentTypename, componentData] : _entityData["Components"].items()) {
        auto comp = targetScene_->componentRepository_->getComponentArray(componentTypename);
        if (!comp) {
            LOG_WARN("Don't Registered Component. Typename {}", componentTypename);
            continue;
        }
        comp->LoadComponent(entity, componentData);
    }
    return entity;
}

GameEntity* SceneSerializer::EntityFromJson(int32_t _entityId, const nlohmann::json& _entityData) {
    std::string entityName = _entityData["Name"];
    bool isUnique          = _entityData["isUnique"];
    int32_t entityID       = targetScene_->entityRepository_->CreateEntity(_entityId, entityName, isUnique);
    if (entityID < 0) {
        LOG_ERROR("Failed to register entity with ID: {}", _entityId);
        return nullptr;
    }
    GameEntity* entity = targetScene_->entityRepository_->getEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->getSystemsRef();
    for (auto& systemData : _entityData["Systems"]) {
        // int32_t systemCategory = systemData["SystemCategory"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = sceneSystems[systemName].get();
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

#pragma endregion
