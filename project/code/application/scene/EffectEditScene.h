#pragma once

//parent
#include "IScene.h"

///stl
#include <memory>

//editor
class EffectManager;

//debugCamera
#include "camera/debugCamera/DebugCamera.h"

class EffectEditScene
    : public IScene {
public:
    EffectEditScene();
    ~EffectEditScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
    std::unique_ptr<DebugCamera> debugCamera_;

    EffectManager* EffectManager_;
};
