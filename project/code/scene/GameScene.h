#pragma once

#include "scene/IScene.h"

#include <list>
#include <memory>
#include <string>

/// Module
class Input;
class DebugCamera;
class GameCamera;

// object
class GameScene
    : public IScene {
public:
    GameScene();
    ~GameScene();

    void Init();
    void Update();

private:
#ifdef _DEBUG
    //DebugObjects
    bool isDebugCameraActive_ = false;
    std::unique_ptr<DebugCamera> debugCamera_;

#endif // _DEBUG
    Input* input_ = nullptr;

    std::unique_ptr<GameCamera> gameCamera_;

};
