#pragma once

#include <memory>

#include "Vector2.h"

class Sprite;

class Enemy {
public:
    Enemy();
    ~Enemy();

    void Init();
    void Update();
    void Draw();
    void OnCollision() { isAlive_ = false; }

private:
    std::unique_ptr<Sprite> sprite_;

    Vector2 velocity_;
    const float speed_ = 48.0f;
    float radius_      = 12.0f;

    bool isAlive_ = false;

public:
    float getRadius() const { return radius_; }
    const Vector2& getPosition() const;

    bool getIsAlive() const { return isAlive_; }
};
