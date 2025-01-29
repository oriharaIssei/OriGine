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
    void Draw() override;

    Vector3f RotateUpdateByStick(float interpolation);

private:
    std::unique_ptr<IPlayerBehavior> currentBehavior_;
    std::unique_ptr<Object3d> shadowObject_;

    std::unique_ptr<AnimationObject3d> effectAnimationObject_;

    CameraTransform* cameraTransform_ = nullptr;

    SerializedField<float> hp_;
    float currentHp_ = 0.0f;

    SerializedField<float> power_;

    bool isInvisible_    = false;
    float invisibleTime_ = 0.0f;

    float jumpForce_ = 0.0f;
    bool onGround_   = true;

    std::unique_ptr<AttackCollider> attackCollider_;

    SerializedField<float> maxMoveLength_;

public:
    bool getOnGround() const { return onGround_; }
    void ChangeBehavior(IPlayerBehavior* next);
    void ChangeBehavior(std::unique_ptr<IPlayerBehavior>& next);

    CameraTransform* getCameraTransform() const { return cameraTransform_; }
    void setCameraTransform(CameraTransform* cameraTransform) { cameraTransform_ = cameraTransform; }

    void resetEffectAnimationObject() {
        effectAnimationObject_.reset();
    }
    void setEffectAnimationObject(std::unique_ptr<AnimationObject3d>& effectAnimationObject) {
        effectAnimationObject_ = std::move(effectAnimationObject);
    }

    void setJumpForce(float force) {
        jumpForce_ = force;
    }
    float getJumpForce() const { return jumpForce_; }

    float getHP() const {
        return currentHp_;
    }
    float getMaxHp() const {
        return hp_;
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

    bool getIsInvisible() const { return isInvisible_; }
    void setInvisibleTime(float time);
    float getInvisibleTime() const { return invisibleTime_; }
};
