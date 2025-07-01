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

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

class SceneManager {
public:
    static SceneManager* getInstance();

    void Initialize();
    void Finalize();

    void Update();

#ifdef _DEBUG
    // void DebugUpdate();
#endif

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&)            = delete;
    SceneManager* operator=(const SceneManager&) = delete;

private:
    SerializedField<std::string> startupSceneName_ = SerializedField<std::string>("Settings", "Scene", "StartupSceneName");

    std::unique_ptr<Scene> currentScene_ = nullptr;

    bool isChangeScene_            = false;
    std::string changingSceneName_ = "";

    bool isExitGame_ = false;

#ifdef _DEBUG
    int32_t playIcon_        = 0;
    int32_t rePlayIcon_      = 0;
    int32_t stopIcon_        = 0;
    int32_t pauseIcon_       = 0;
    int32_t pauseCircleIcon_ = 0;

    int32_t cameraIcon_ = 0;

    bool isUsingDebugCamera_ = false;

    char newSceneName_[256] = "NewScene";

private:
    enum class SceneState {
        Edit,
        Debug
    };
    SceneState currentSceneState_ = SceneState::Edit;

    enum class DebugState {
        Play,
        Pause,
        Stop,
        RePlay
    };
    DebugState debugState_ = DebugState::Stop;

#endif

public:
    Scene* getCurrentScene() {
        return currentScene_.get();
    }
    bool isExitGame() const {
        return isExitGame_;
    }
    void setExitGame(bool exit) { isExitGame_ = exit; }

    const std::string& getCurrentSceneName() const;

    void sceneChange2StartupScene();
    void changeScene(const std::string& name);

    void executeSceneChange();

    bool isChangeScene() const { return isChangeScene_; }
};

class SceneSerializer {
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
    GameEntity* EntityFromJson(int32_t _entityId,const nlohmann::json& _entityData);

private:
    static const std::string directory_;
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
