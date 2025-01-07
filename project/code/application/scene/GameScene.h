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
class Object3d;
class Sprite;

// object
class GameObject;
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
    Input* input_;

    std::unique_ptr<GameCamera> gameCamera_;

    std::vector<GameObject*> activeGameObjects_;
    std::unique_ptr<CollisionManager> collisionManager_;

    std::unique_ptr<Player> player_;
    std::unique_ptr<EnemyManager> enemyManager_;

    std::unique_ptr<Object3d> ground_;
    std::unique_ptr<Object3d> skyDome_;

    std::unique_ptr<Sprite> dashUI_ = nullptr;
    std::unique_ptr<Sprite> attackUI_ = nullptr;
    std::unique_ptr<Sprite> jumpUI_ = nullptr;

    std::unique_ptr<PlayerHpBar> playerHpBar_;

    SerializedField<Vector2> dashUIPos_;
    SerializedField<Vector2> attackUIPos_;
    SerializedField<Vector2> jumpUIPos_;

};
