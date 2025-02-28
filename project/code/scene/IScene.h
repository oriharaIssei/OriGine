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

    virtual void Init()   = 0;
    virtual void Update() = 0;

protected:
    std::shared_ptr<DxRtvArray> sceneRtvArray_;
    std::shared_ptr<DxSrvArray> sceneSrvArray_;

private:
    std::string name_;
};
