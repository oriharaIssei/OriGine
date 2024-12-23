#pragma once

struct Vector2;
class Sprite;
class GameScene;

#include <memory>

class Player{

public:
    Player();
    ~Player();

    void Init();
    void Update(GameScene* _hostScene);
    void Draw();
    void OnCollision(){ isAlive_ = false; }

private:
    std::unique_ptr<Sprite> sprite_;

    const float speed_         = 64.0f;
    const float radius_        = 8.0f;
    const float shotCoolTime_  = 0.6f;
    float shotCoolTimeCounter_ = 0.0f;

    bool isAlive_ = false;
public:
    const Vector2& getPosition() const;
    const float getRadius() const{ return radius_; }
    bool getIsAlive() const{ return isAlive_; }
};
