#pragma once
#include "IScene.h"

#include <memory>

class Input;
class Sprite;

class GameClearScene :
    public IScene{
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
#ifdef _DEBUG
    // DebugObjects

#endif // _DEBUG

    Input* input_;

    std::unique_ptr<Sprite> clearSprite_;
    std::unique_ptr<Sprite> pushSpaceSprite_;
};
