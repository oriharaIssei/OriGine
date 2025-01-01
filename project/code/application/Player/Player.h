#pragma once

#include "globalVariables/SerializedField.h"
#include "Transform/Transform.h"

#include <memory>

class IPlayerBehavior;
class Object3d;

class Player {
public:
    Player();
    ~Player();

    void Init();
    void Update();
    void Draw();

private:
    std::unique_ptr<IPlayerBehavior> currentBehavior_;

    std::unique_ptr<Object3d> drawObject3d_;
    Transform transform_;


    SerializedField<float> hp_;
    float currentHp_ = 0.0f;

public:
    void ChangeBehavior(IPlayerBehavior* next);

    const Vector3& getScale() const;
    const Quaternion& getRotate() const;
    const Vector3& getTranslate() const;

    void setScale(const Vector3& s);
    void setRotate(const Quaternion& q);
    void setTranslate(const Vector3& t);

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
