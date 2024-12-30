#pragma once

#include "IScene.h"

#include <list>
#include <memory>
#include <string>

class Input;
class MaterialManager;
class DebugCamera;
class Object3d;

class AnimationEditor;

class GameScene
    : public IScene {
public:
    GameScene();
    ~GameScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
#ifdef _DEBUG
    //DebugObjects
    std::unique_ptr<DebugCamera> debugCamera_;

    std::unique_ptr<AnimationEditor> animationEditor_;
#endif // _DEBUG

    Input* input_;
    MaterialManager* materialManager_;

    std::unique_ptr<Object3d> ground_;
    std::unique_ptr<Object3d> skyDome_;
};
