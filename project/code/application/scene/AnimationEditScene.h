#pragma once
#include "IScene.h"

#include "animationEditor/AnimationEditor.h"
#include "camera/debugCamera/DebugCamera.h"

class AnimationEditScene
    : public IScene{
public:
    AnimationEditScene();
    ~AnimationEditScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
    //DebugObjects
    std::unique_ptr<DebugCamera> debugCamera_;
    std::unique_ptr<AnimationEditor> animationEditor_;
};
