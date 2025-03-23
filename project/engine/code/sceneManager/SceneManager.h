#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>
// string
#include <string>

class IScene;

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class EntityComponentSystemManager;
class EngineEditor;

class SceneManager {
    friend class EngineEditor;

public:
    static SceneManager* getInstance();

    void Initialize();
    void Finalize();

    void Update();
    void Draw();

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&)            = delete;
    SceneManager* operator=(const SceneManager&) = delete;

private:
    std::unique_ptr<IScene> currentScene_ = nullptr;

    bool isChangeScene_              = false;
    std::string changingSceneName_ = "";

    EntityComponentSystemManager* ecsManager_ = nullptr;

    std::unique_ptr<RenderTexture> sceneView_;
    std::shared_ptr<DxRtvArray> sceneViewRtvArray_;
    std::shared_ptr<DxSrvArray> sceneViewSrvArray_;

    std::unordered_map<std::string, int32_t> sceneIndexs_;
    std::vector<std::function<std::unique_ptr<IScene>()>> scenes_;
public:
    IScene* getCurrentScene() const { return currentScene_.get(); }
    void addScene(const std::string& name, std::function<std::unique_ptr<IScene>()> _sceneMakeFunc);
    void changeScene(const std::string& name);

    RenderTexture* getSceneView() const { return sceneView_.get(); }

    const std::unordered_map<std::string, int32_t>& getScenes() const { return sceneIndexs_; }
};
