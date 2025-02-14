#pragma once

#include "IScene.h"

#include <list>
#include <memory>
#include <string>

/// Module
class Input;
class MaterialManager;
class DebugCamera;
class CollisionManager;

// component
#include "sprite/Sprite.h"

// object
class EnemyManager;
class Player;
class IEnemy;
class GameCamera;
class PlayerHpBar;

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
    bool isDebugCameraActive_ = false;
    std::unique_ptr<DebugCamera> debugCamera_;

#endif // _DEBUG
    Input* input_ = nullptr;

    std::shared_ptr<SpriteRenderer> test;

    std::unique_ptr<GameCamera> gameCamera_;

};
