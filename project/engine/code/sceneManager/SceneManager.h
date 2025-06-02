#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>
// string
#include <string>

/// Engine
#include "module/editor/IEditor.h"

/// lib
#include "lib/globalVariables/SerializedField.h"

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

class IScene;

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class EntityComponentSystemManager;
class GameEntity;

#ifdef _DEBUG
class EditorGroup;
class DebuggerGroup;
#endif // _DEBUG

class SceneManager {
public:
    static SceneManager* getInstance();

    void Initialize();
    void Finalize();

    void Update();

#ifdef _DEBUG
    void DebugUpdate();
#endif

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&)            = delete;
    SceneManager* operator=(const SceneManager&) = delete;

private:
    SerializedField<std::string> startupSceneName_ = SerializedField<std::string>("Settings", "Scene", "StartupSceneName");

    std::string currentSceneName_  = "MyGame";
    bool isChangeScene_            = false;
    std::string changingSceneName_ = "";

    EntityComponentSystemManager* ecsManager_ = nullptr;

    /// SceneView
    std::unique_ptr<RenderTexture> sceneView_;

    std::shared_ptr<DxRtvArray> sceneViewRtvArray_;
    std::shared_ptr<DxSrvArray> sceneViewSrvArray_;

    bool isExitGame_ = false;

#ifdef _DEBUG
    EditorGroup* editorGroup_     = nullptr;
    DebuggerGroup* debuggerGroup_ = nullptr;

    int32_t playIcon_        = 0;
    int32_t rePlayIcon_      = 0;
    int32_t stopIcon_        = 0;
    int32_t pauseIcon_       = 0;
    int32_t pauseCircleIcon_ = 0;

    int32_t cameraIcon_      = 0;
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
    bool isExitGame() const { return isExitGame_; }
    void setExitGame(bool exit) { isExitGame_ = exit; }

    const std::string& getCurrentSceneName() const { return currentSceneName_; }

    void sceneChange2StartupScene();
    void changeScene(const std::string& name);

    void executeSceneChange();

    RenderTexture* getSceneView() const { return sceneView_.get(); }

    bool isChangeScene() const { return isChangeScene_; }

#ifdef _DEBUG
    bool inDebugMode() const {
        return currentSceneState_ == SceneState::Debug;
    };
    bool inEditMode() const {
        return currentSceneState_ == SceneState::Edit;
    };

    bool debugIsPlay() const {
        return debugState_ == DebugState::Play;
    };
    bool debugIsPause() const {
        return debugState_ == DebugState::Pause;
    };
    bool debugIsStop() const {
        return debugState_ == DebugState::Stop;
    };
    bool debugIsRePlay() const {
        return debugState_ == DebugState::RePlay;
    };

    bool isUsingDebugCamera() const {
        return isUsingDebugCamera_;
    };
#endif // _DEBUG
};

class SceneSerializer {
public:
    SceneSerializer() {}
    ~SceneSerializer() {}

    /// <summary>
    /// シーンを保存する(基本はこれを使う)
    /// </summary>
    bool Serialize(const std::string& _sceneName);

    /// <summary>
    /// シーンを読み込む(基本はこれを使う)
    /// </summary>
    void Deserialize(
        const std::string& _sceneName);

    /// <summary>
    /// シーンを保存する(警告なし)
    /// </summary>
    void SerializeFromJson(const std::string& _sceneName);

    /// <summary>
    /// シーンを読み込む
    /// </summary>
    void DeserializeFromJson(const std::string& _sceneName);

    void SaveEntity(GameEntity* _entity, const std::string& _directory);
    GameEntity* LoadEntity(const std::string& _directory, const std::string& _dataType);

private:
    static const std::string directory_;
    nlohmann::json rootJson_;

public:
    const nlohmann::json& getRootJson() const {
        return rootJson_;
    }

    void setRootJson(const nlohmann::json& _rootJson) {
        rootJson_ = _rootJson;
    }
};
