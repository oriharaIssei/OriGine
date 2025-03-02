#pragma once

#include "IScene.h"

#include <list>
#include <memory>
#include <string>

// object
class GameScene
    : public IScene {
public:
    GameScene();
    ~GameScene();

    void Init();
    void Finalize() override;

protected:
    void CreateSceneEntity() override;
    void CreateSceneSystem() override;
    void LoadSceneEntity() override;
    void SaveSceneEntity() override;

private:
};
