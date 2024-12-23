#pragma once

#include "Vector2.h"

#include <memory>

class Sprite;

class Bullet {
public:
    Bullet();
    ~Bullet();

    void Init(const Vector2& _pos, const Vector2& _direction);
    void Update();
    void Draw();
    void OnCollision() { isAlive_ = false; }

private:
    std::unique_ptr<Sprite> sprite_;

    float radius_ = 3.0f;

    const float speed_ = 128.0f;
    Vector2 velocity_;

    bool isAlive_ = false;

public:
    bool getIsAlive() const { return isAlive_; }
    float getRadius() const { return radius_; }

    const Vector2& getPosition() const ;
};
