#include "SceneManager.h"

// Interface
#include "scene/Scene.h"

/// stl
#include <format>

/// engine
#include "Engine.h"
#include "logger/Logger.h"
#include "winApp/WinApp.h"
// input
#include "input/GamepadInput.h"
#include "input/InputManager.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

#define ENGINE_INPUT
#define RESOURCE_DIRECTORY
#define DELTA_TIME
#include "EngineInclude.h"
// ecs
#include "component/ComponentArray.h"
#include "entity/Entity.h"
#include "system/ISystem.h"
#include "system/SystemRunner.h"

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

SceneManager::SceneManager() {}
SceneManager::~SceneManager() {}

void SceneManager::Initialize(const std::string& _startScene, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    // 入力デバイスの設定
    keyInput_   = _keyInput;
    mouseInput_ = _mouseInput;
    padInput_   = _padInput;

    // シーンの初期化
    currentScene_ = std::make_unique<Scene>(_startScene);
    // シーンの入力デバイスを設定
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // シーンの初期化処理
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーンマネージャーの設定 (this)
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    fileWatcher_ = std::make_unique<FileWatcher>(kApplicationResourceDirectory + "/scene/" + _startScene + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP
}
void SceneManager::Initialize(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    this->Initialize(startupSceneName_, _keyInput, _mouseInput, _padInput);
}

void SceneManager::Finalize() {
#ifdef _DEVELOP
    fileWatcher_->Stop();
#endif // _DEVELOP
}

void SceneManager::Update() {
#ifdef _DEVELOP
    if (fileWatcher_->isChanged()) {
        this->ChangeScene(currentScene_->GetName());
    }
#endif // _DEVELOP

    currentScene_->Update();
}

void SceneManager::Render() {
    currentScene_->Render();
}

const std::string& SceneManager::GetCurrentSceneName() const { return currentScene_->GetName(); }

void SceneManager::ChangeScene(const std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}

void SceneManager::ExecuteSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentScene_->GetName(), changingSceneName_);

    currentScene_->Finalize();
    currentScene_ = std::make_unique<Scene>(changingSceneName_);

    // 入力デバイスの設定
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // シーンの初期化処理
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーンマネージャーの設定 (this)
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    // 監視対象を変更
    fileWatcher_->Stop();
    fileWatcher_->SetFilePath(kApplicationResourceDirectory + "/scene/" + changingSceneName_ + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP

    isChangeScene_ = false;
}
#pragma endregion

#pragma region "SceneSerializer"

const std::string SceneSerializer::kSceneDirectory = kApplicationResourceDirectory + "/scene/";

SceneSerializer::SceneSerializer(Scene* _targetScene) : targetScene_(_targetScene) {}

SceneSerializer::~SceneSerializer() {}

bool SceneSerializer::Serialize() {
    if (!targetScene_) {
        LOG_ERROR("Target scene is null");
        return false;
    }
    std::string message = std::format("{} save it?", targetScene_->GetName());

    if (MessageBoxA(nullptr, message.c_str(), "SceneSerializer", MB_OKCANCEL) != IDOK) {
        return false;
    }

    // 保存
    SerializeFromJson();

    message = std::format("{} saved", targetScene_->GetName());
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

    std::list<Entity*> aliveEntities;

    for (auto& entity : entities->GetEntitiesRef()) {
        if (entity.IsAlive() && entity.ShouldSave()) {
            aliveEntities.push_back(&entity);
        }
    }

    for (auto& entity : aliveEntities) {
        if (!entity->IsAlive()) {
            continue;
        }
        nlohmann::json entityData = nlohmann::json::object();

        entityData = EntityToJson(entity->GetID());

        jsonData["Entities"].push_back(entityData);
    }

    /// =====================================================
    // System
    /// =====================================================
    {
        const auto& systems                 = targetScene_->systemRunner_->GetSystems();
        nlohmann::json systemsData          = nlohmann::json::array();
        nlohmann::json systemDataByCategory = nlohmann::json::object();

        for (const auto& [name, system] : systems) {
            if (!system || !system->IsActive()) {
                continue; // 無効なシステムはスキップ
            }
            nlohmann::json systemData  = nlohmann::json::object();
            systemData["Priority"]     = system->GetPriority();
            systemDataByCategory[name] = systemData;
        }
        systemsData.push_back(systemDataByCategory);

        jsonData["Systems"] = systemsData;

        nlohmann::json categoryActivity = nlohmann::json::array();
        for (size_t i = 0; i < static_cast<size_t>(SystemCategory::Count); ++i) {
            categoryActivity.push_back(targetScene_->systemRunner_->GetCategoryActivityRef()[i]);
        }
        jsonData["CategoryActivity"] = categoryActivity;
    }

    // JSON ファイルに書き込み
    std::string sceneFilePath = kSceneDirectory + targetScene_->GetName() + ".json";
    myfs::DeleteMyFile(sceneFilePath);
    myfs::CreateFolder(kSceneDirectory);
    std::ofstream ofs(sceneFilePath);
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", targetScene_->GetName());
        return;
    }
    ofs << std::setw(4) << jsonData << std::endl;
    ofs.close();
}

void SceneSerializer::DeserializeFromJson() {
    std::ifstream ifs(kSceneDirectory + targetScene_->GetName() + ".json");
    if (!ifs) {
        LOG_ERROR("Failed to open JSON file for reading: {}", targetScene_->GetName());
        return;
    }

    nlohmann::json jsonData;
    ifs >> jsonData;
    ifs.close();

    auto& systemRunner = targetScene_->systemRunner_;

    /// =====================================================
    // System
    /// =====================================================
    int32_t systemCategoryIndex = 0;
    nlohmann::json& systems     = jsonData["Systems"];
    for (auto& systemByType : systems) {
        for (auto& [systemName, system] : systemByType.items()) {
            systemRunner->RegisterSystem(systemName, system["Priority"], true);
        }
        ++systemCategoryIndex;
    }

    nlohmann::json& systemCategoryActivities = jsonData["CategoryActivity"];
    for (int32_t category = 0; category < static_cast<int32_t>(SystemCategory::Count); ++category) {
        if (category < systemCategoryActivities.size()) {
            bool isActive = systemCategoryActivities[category].get<bool>();
            systemRunner->SetCategoryActivity(static_cast<SystemCategory>(category), isActive);
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
}

void SceneSerializer::SaveEntity(int32_t _entityID, const std::string& _directory) {
    Entity* _entity = targetScene_->entityRepository_->GetEntity(_entityID);

    if (!_entity || !_entity->IsAlive()) {
        return;
    }

    nlohmann::json entityData;
    entityData = EntityToJson(_entityID);

    // ディレクトリを作成
    myFs::CreateFolder(_directory);
    std::string filePath = _directory + "/" + _entity->GetDataType() + ".ent";
    // JSONファイルに書き込み
    std::ofstream ofs(filePath);
    if (!ofs) {
        LOG_ERROR("Failed to open JSON file for writing: {}", filePath);
        return;
    }
    ofs << std::setw(4) << entityData << std::endl;
    ofs.close();
}

nlohmann::json SceneSerializer::EntityToJson(int32_t _entityID) {
    nlohmann::json entityData = nlohmann::json::object();

    Entity* _entity = targetScene_->entityRepository_->GetEntity(_entityID);
    if (!_entity || !_entity->IsAlive()) {
        // 無効なエンティティの場合、空のJSONオブジェクトを返す
        return entityData;
    }

    entityData["Name"]     = _entity->GetDataType();
    entityData["isUnique"] = _entity->IsUnique();

    // 所属するシステムを保存
    entityData["Systems"]    = nlohmann::json::array();
    auto& systemsJson        = entityData["Systems"];
    const auto& sceneSystems = targetScene_->systemRunner_->GetSystems();
    for (const auto& [systemName, system] : sceneSystems) {
        if (!system) {
            LOG_WARN("System not found: {}", systemName);
            continue; // 無効なシステムはスキップ
        }
        if (system->HasEntity(_entity)) {
            systemsJson.push_back({{"SystemCategory", system->GetCategory()}, {"SystemName", systemName}});
        }
    }

    // コンポーネントを保存
    const auto& componentArrayMap = targetScene_->componentRepository_->GetComponentArrayMap();
    nlohmann::json componentsData;
    for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
        if (componentArray->HasEntity(_entity)) {
            componentArray->SaveComponent(_entity, componentsData);
        }
    }
    entityData["Components"] = componentsData;

    return entityData;
}

Entity* SceneSerializer::LoadEntity(const std::string& _directory, const std::string& _dataType) {
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

Entity* SceneSerializer::EntityFromJson(const nlohmann::json& _entityData) {
    std::string entityName = _entityData.at("Name");
    bool isUnique          = _entityData.at("isUnique");
    int32_t entityID       = targetScene_->entityRepository_->CreateEntity(entityName, isUnique);
    Entity* entity         = targetScene_->entityRepository_->GetEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->GetSystemsRef();
    for (auto& systemData : _entityData.at("Systems")) {
        // int32_t systemCategory = systemData.at("SystemCategory");
        std::string systemName = systemData.at("SystemName");
        ISystem* system        = sceneSystems[systemName].get();
        if (system) {
            system->AddEntity(entity);
        }
    }
    // コンポーネントを読み込み
    for (auto& [componentTypename, componentData] : _entityData["Components"].items()) {
        auto comp = targetScene_->componentRepository_->GetComponentArray(componentTypename);
        if (!comp) {
            LOG_WARN("Don't Registered Component. Typename {}", componentTypename);
            continue;
        }
        comp->LoadComponent(entity, componentData);
    }
    return entity;
}

Entity* SceneSerializer::EntityFromJson(int32_t _entityId, const nlohmann::json& _entityData) {
    std::string entityName = _entityData["Name"];
    bool isUnique          = _entityData["isUnique"];
    int32_t entityID       = targetScene_->entityRepository_->CreateEntity(_entityId, entityName, isUnique);
    if (entityID < 0) {
        LOG_ERROR("Failed to register entity with ID: {}", _entityId);
        return nullptr;
    }
    Entity* entity = targetScene_->entityRepository_->GetEntity(entityID);

    // 所属するシステムを読み込み
    auto& sceneSystems = targetScene_->systemRunner_->GetSystemsRef();
    for (auto& systemData : _entityData["Systems"]) {
        // int32_t systemCategory = systemData["SystemCategory"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = sceneSystems[systemName].get();
        if (system) {
            system->AddEntity(entity);
        }
    }
    // コンポーネントを読み込み
    auto& componentArrayMap = targetScene_->componentRepository_->GetComponentArrayMapRef();
    for (auto& [componentTypename, componentData] : _entityData["Components"].items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }
    return entity;
}

#pragma endregion
