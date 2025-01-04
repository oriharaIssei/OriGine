#pragma once

///stl
//memory
#include <memory>

///engine
//component
#include "../GameObject/GameObject.h"
#include "Transform/Transform.h"
#include "transform/CameraTransform.h"
class IPlayerBehavior;
class Object3d;
class Collider;
//lib
#include "globalVariables/SerializedField.h"

class Player
    : public GameObject {
public:
    Player();
    ~Player();

    void Init();
    void Update();

private:
    std::unique_ptr<IPlayerBehavior> currentBehavior_;

    CameraTransform* cameraTransform_ = nullptr;

    SerializedField<float> hp_;
    float currentHp_ = 0.0f;

    std::unique_ptr<Collider> hitCollider_;

public:
    void ChangeBehavior(IPlayerBehavior* next);

    CameraTransform* getCameraTransform() const { return cameraTransform_; }
    void setCameraTransform(CameraTransform* cameraTransform) { cameraTransform_ = cameraTransform; }

    Collider* getHitCollider() const { return hitCollider_.get(); }

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
