#pragma once

#include "../GameObject/GameObject.h"
#include "globalVariables/SerializedField.h"
#include "Transform/Transform.h"

#include <memory>

class IPlayerBehavior;
class Object3d;

class Player
    : public GameObject {
public:
    Player();
    ~Player();

    void Init();
    void Update();
    void Draw();

private:
    std::unique_ptr<IPlayerBehavior> currentBehavior_;

    SerializedField<float> hp_;
    float currentHp_ = 0.0f;

public:
    void ChangeBehavior(IPlayerBehavior* next);

    float getHP() const {
        return currentHp_;
    }
    void setHP(float hp) {
        currentHp_ = hp;
    }
    void Damage(float damage) {
        currentHp_ -= damage;
    }
};
