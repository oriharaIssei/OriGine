#pragma once

#include <memory>
#include <unordered_map>

#include <string>

class IScene;

class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class EngineEditor;

class SceneManager{
    friend class EngineEditor;
public:
    static SceneManager* getInstance();

    void Init();
    void Finalize();

    void Update();
    void Draw();
private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&) = delete;
    SceneManager* operator=(const SceneManager&) = delete;
private:
    IScene* currentScene_ = nullptr;

    std::unique_ptr<RenderTexture> sceneView_;
    std::shared_ptr<DxRtvArray> sceneViewRtvArray_;
    std::shared_ptr<DxSrvArray> sceneViewSrvArray_;

    std::unordered_map<std::string,int32_t> sceneIndexs_;
    std::vector<std::unique_ptr<IScene>> scenes_;
public:
    void addScene(const std::string& name,std::unique_ptr<IScene> scene);
    void changeScene(const std::string& name);
};
