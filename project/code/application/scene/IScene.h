#pragma once

/// stl
#include <memory>
#include <string>

/// engine
class RenderTexture;
class DxRtvArray;
class DxSrvArray;

class IScene {
public:
    IScene(const std::string& sceneName);
    virtual ~IScene();

    virtual void Init();
    virtual void Finalize();

protected:
    virtual void CreateSceneEntity() = 0;
    virtual void CreateSceneSystem() = 0;

    virtual void LoadSceneEntity() = 0;
    virtual void SaveSceneEntity() = 0;

protected:
    std::shared_ptr<DxRtvArray> sceneRtvArray_;
    std::shared_ptr<DxSrvArray> sceneSrvArray_;

private:
    std::string name_;

public:
    const std::string& GetName() const {
        return name_;
    }
    std::shared_ptr<DxRtvArray> GetSceneRtvArray() const {
        return sceneRtvArray_;
    }
    std::shared_ptr<DxSrvArray> GetSceneSrvArray() const {
        return sceneSrvArray_;
    }
};
