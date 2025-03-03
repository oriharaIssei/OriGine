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
    void LoadSceneEntity() override;
    void SaveSceneEntity() override;

private:
};
