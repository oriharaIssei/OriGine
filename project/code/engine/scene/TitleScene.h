#pragma once
#include "IScene.h"

///stl
#include <functional>
#include <memory>

//module
class Input;
//component
class Sprite;

class TitleScene
    : public IScene {
public:
    TitleScene();
    ~TitleScene();

    void Init();
    void Update();

    void Draw3d();
    void DrawLine();
    void DrawSprite();
    void DrawParticle();

private:
    Input* input_ = nullptr;

    float time_;

    std::unique_ptr<Sprite> title_;
    std::unique_ptr<Sprite> pushA_;
};
