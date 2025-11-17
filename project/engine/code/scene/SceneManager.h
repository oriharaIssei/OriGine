#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>
// string
#include <string>

/// engine
class Scene;
// input
class KeyboardInput;
class MouseInput;
class GamePadInput;
/// ecs
#include "component/ComponentArray.h"
#include "entity/Entity.h"

/// util
#include "util/globalVariables/SerializedField.h"
#include "util/myFileSystem/MyFileSystem.h"

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

class SceneManager {
    friend class ReplayPlayer;

public:
    SceneManager();
    ~SceneManager();

    void Initialize(const std::string& _startScene, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamePadInput* _padInput);
    void Initialize(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamePadInput* _padInput);
    void Finalize();

    void Update();
    void Render();

    /// <summary>
    /// 即座にシーンを変更する(事前にnextSceneを指定する必要がある)
    /// </summary>
    void executeSceneChange();

private:
#ifdef _DEVELOP
    // Editorでの変更を検知する
    std::unique_ptr<FileWatcher> fileWatcher_ = nullptr;
#endif

    SerializedField<std::string> startupSceneName_ = SerializedField<std::string>("Settings", "Scene", "StartupSceneName");

    std::unique_ptr<Scene> currentScene_ = nullptr;

    // input
    KeyboardInput* keyInput_ = nullptr;
    MouseInput* mouseInput_  = nullptr;
    GamePadInput* padInput_  = nullptr;

    std::string changingSceneName_ = "";

    bool isChangeScene_ = false;
    bool isExitGame_    = false;

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

    Entity* LoadEntity(const std::string& _directory, const std::string& _dataType);
    Entity* EntityFromJson(const nlohmann::json& _entityData);
    Entity* EntityFromJson(int32_t _entityId, const nlohmann::json& _entityData);

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
