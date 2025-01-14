#pragma once

//parent
#include "IScene.h"

///stl
#include <memory>

//editor
class ParticleManager;

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
    ParticleManager* particleManager_;
};
