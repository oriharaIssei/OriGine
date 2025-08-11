#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>
// string
#include <string>

/// engine
#include "component/ComponentArray.h"
#include "ECS/Entity.h"
#include "Scene.h"
/// lib
#include "lib/globalVariables/SerializedField.h"
#include "lib/myFileSystem/MyFileSystem.h"

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

class SceneManager {
public:
    static SceneManager* getInstance();

    void Initialize(const std::string& _startScene);
    void Initialize();
    void Finalize();

    void Update();

    /// <summary>
    /// 即座にシーンを変更する(事前にnextSceneを指定する必要がある)
    /// </summary>
    void executeSceneChange();

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&)            = delete;
    SceneManager* operator=(const SceneManager&) = delete;

private:
#ifdef _DEVELOP
    // Editorでの変更を検知する
    std::unique_ptr<FileWatcher> fileWatcher_ = nullptr;
#endif

    SerializedField<std::string> startupSceneName_ = SerializedField<std::string>("Settings", "Scene", "StartupSceneName");

    std::unique_ptr<Scene> currentScene_ = nullptr;

    bool isChangeScene_            = false;
    std::string changingSceneName_ = "";

    bool isExitGame_ = false;

public:
    Scene* getCurrentScene() {
        return currentScene_.get();
    }

    const SerializedField<std::string>& getStartupSceneName() const {
        return startupSceneName_;
    }
    SerializedField<std::string>& getStartupSceneNameRef() {
        return startupSceneName_;
    }

    bool isExitGame() const {
        return isExitGame_;
    }
    void setExitGame(bool exit) { isExitGame_ = exit; }

    const std::string& getCurrentSceneName() const;

    void changeScene(const std::string& name);

    bool isChangeScene() const { return isChangeScene_; }
};

class SceneSerializer {
public:
    static const std::string SceneDirectory;

public:
    SceneSerializer(Scene* _targetScene);
    ~SceneSerializer();
    /// <summary>
    /// シーンを保存する(基本はこれを使う)
    /// </summary>
    bool Serialize();

    /// <summary>
    /// シーンを読み込む(基本はこれを使う)
    /// </summary>
    void Deserialize();

    /// <summary>
    /// シーンを保存する(警告なし)
    /// </summary>
    void SerializeFromJson();

    /// <summary>
    /// シーンを読み込む
    /// </summary>
    void DeserializeFromJson();

    void SaveEntity(int32_t _entityID, const std::string& _directory);
    void EntityToJson(int32_t _entityID, nlohmann::json& entityData);

    GameEntity* LoadEntity(const std::string& _directory, const std::string& _dataType);
    GameEntity* EntityFromJson(const nlohmann::json& _entityData);
    GameEntity* EntityFromJson(int32_t _entityId, const nlohmann::json& _entityData);

private:
    nlohmann::json rootJson_;

    Scene* targetScene_ = nullptr;

public:
    const nlohmann::json& getRootJson() const {
        return rootJson_;
    }

    void setRootJson(const nlohmann::json& _rootJson) {
        rootJson_ = _rootJson;
    }
};

/// <summary>
/// !!!非推奨!!!
/// currentSceneの指定したEntityを取得
/// </summary>
/// <param name="_entityID"></param>
/// <returns></returns>
GameEntity* getSceneEntity(int32_t _entityID);
/// <summary>
/// !!!非推奨!!!
/// currentSceneの指定した UniqueEntityを取得
/// </summary>
/// <param name="_name"></param>
/// <returns></returns>
GameEntity* getSceneUniqueEntity(const std::string& _name);

template <IsComponent CompType>
CompType* getSceneComponent(GameEntity* _entity, int32_t _compIdx = 0) {
    if (!_entity) {
        return nullptr;
    }
    Scene* currentScene = SceneManager::getInstance()->getCurrentScene();

    return currentScene->getComponentRepositoryRef()->getComponent<CompType>(_entity, _compIdx);
}
