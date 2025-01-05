#pragma once

///stl
//memory
#include <memory>

///engine
//component
#include "transform/CameraTransform.h"
#include "transform/Transform.h"
class IPlayerBehavior;
//object
#include "../AttackCollider/AttackCollider.h"
#include "../GameObject/GameObject.h"
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

    SerializedField<float> power_;

    bool isInvisible_    = false;
    float invisibleTime_ = 0.0f;

    std::unique_ptr<Collider> hitCollider_;
    std::unique_ptr<AttackCollider> attackCollider_;

public:
    void ChangeBehavior(IPlayerBehavior* next);
    void ChangeBehavior(std::unique_ptr<IPlayerBehavior>& next);

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

    float getPower() const {
        return power_;
    }

    AttackCollider* getAttackCollider() const { return attackCollider_.get(); }
    void setAttackCollider(std::unique_ptr<AttackCollider>& attackCollider) {
        attackCollider_ = std::move(attackCollider);
    }
    void resetAttackCollider() {
        attackCollider_.reset();
    }

    void setInvisibleTime(float time) { invisibleTime_ = time; }
    float getInvisibleTime() const { return invisibleTime_; }
};
