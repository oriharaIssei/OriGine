#pragma once

//parent
#include "IScene.h"

///stl
#include <memory>

//editor
class ParticleEditor;
class EffectEditor;

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

    std::unique_ptr<EffectEditor> effectEditor_;
    std::unique_ptr<ParticleEditor> particleEditor_;
};
