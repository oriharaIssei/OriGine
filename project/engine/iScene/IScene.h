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

    virtual void LoadSceneEntity();
    virtual void SaveSceneEntity();

protected:
    /// <summary>
    /// シーン内で使用するComponentを登録する
    /// </summary>
    virtual void registerComponents();
    /// <summary>
    /// シーン内で使用する System を登録する
    /// </summary>
    virtual void registerSystems();

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
