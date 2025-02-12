#pragma once
#include "IScene.h"

///stl
#include <functional>
#include <memory>

//module
class Input;
//component
class Sprite;

class GameClearScene
    : public IScene {
public:
    GameClearScene();
    ~GameClearScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
    Input* input_ = nullptr;

    float time_ = 0.0f;

    std::unique_ptr<Sprite> text_;
    std::unique_ptr<Sprite> pushA_;
};
