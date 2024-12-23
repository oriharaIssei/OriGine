#pragma once

#include "IScene.h"

#include <memory>
#include <string>
#include <vector>

class Input;
class MaterialManager;

class Collision;

// Object
class Player;
class Enemy;
class Bullet;

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
    // DebugObjects

#endif // _DEBUG

    Input* input_;
    MaterialManager* materialManager_;

    std::unique_ptr<Player> player_;
    std::vector<std::unique_ptr<Enemy>> enemyList_;
    std::vector<std::unique_ptr<Bullet>> bulletList_;

    std::unique_ptr<Collision> collision_;

public:
    void addBullet(std::unique_ptr<Bullet> bullet);
};
