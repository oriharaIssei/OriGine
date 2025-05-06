#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>
// string
#include <string>

/// Engine
#include "module/editor/IEditor.h"

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

class IScene;

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class EntityComponentSystemManager;
class EditorGroup;
class DebuggerGroup;

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
    std::unique_ptr<IScene> currentScene_ = nullptr;

    bool isChangeScene_            = false;
    std::string changingSceneName_ = "";

    EntityComponentSystemManager* ecsManager_ = nullptr;

    /// SceneView
    std::unique_ptr<RenderTexture> sceneView_;

    std::shared_ptr<DxRtvArray> sceneViewRtvArray_;
    std::shared_ptr<DxSrvArray> sceneViewSrvArray_;

    std::unordered_map<std::string, int32_t> sceneIndexs_;
    std::vector<std::function<std::unique_ptr<IScene>()>> scenes_;

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
    IScene* getCurrentScene() const { return currentScene_.get(); }
    void addScene(const std::string& name, std::function<std::unique_ptr<IScene>()> _sceneMakeFunc);
    void changeScene(const std::string& name);

    void executeSceneChange();

    RenderTexture* getSceneView() const { return sceneView_.get(); }

    const std::unordered_map<std::string, int32_t>& getScenes() const { return sceneIndexs_; }

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

    void Serialize(const std::string& _sceneName);

    void Deserialize(
        const std::string& _sceneName) {
        DeserializeFromJson(_sceneName);
    }

private:
    void SerializeFromJson(const std::string& _sceneName);
    void DeserializeFromJson(const std::string& _sceneName);

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
