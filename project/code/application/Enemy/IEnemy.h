#pragma once

/// stl
#include <memory>
#include <string>

/// engine
//component
#include "transform/Transform.h"
//lib
#include "globalVariables/SerializedField.h"

/// application
//Behavior
#include "BehaviorTree/DefaultNodes.h"
//component
#include "application/Collision/Collider.h"
//object
#include "../AttackCollider/AttackCollider.h"
#include "../GameObject/GameObject.h"
class Collider;
class Player;
//math
#include "Quaternion.h"
#include "Vector3.h"

class IEnemy
    : public GameObject {
public:
    IEnemy(const std::string& _type);
    virtual ~IEnemy();

    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Draw();

    virtual std::unique_ptr<IEnemy> Clone() = 0;

protected:
    Player* player_ = nullptr;

    std::unique_ptr<Collider> hitCollider_;

    std::unique_ptr<AttackCollider> attackCollider_;

    std::unique_ptr<EnemyBehavior::Node> behaviorTree_ = nullptr;

    SerializedField<float> hp_;
    SerializedField<float> speed_;
    SerializedField<float> attack_;
    float currentHp_     = 0.0f;
    float currentSpeed_  = 0.0f;
    float currentAttack_ = 0.0f;

    SerializedField<float> maxMoveLenght_;

    bool isInvisible_    = true;
    float invisibleTime_ = 0.0f;

public:
    Player* getPlayer() { return player_; }
    void setPlayer(Player* player) {
        player_ = player;
    }

    Collider* getHitCollider() const {
        return hitCollider_.get();
    }

    float getHp() const {
        return hp_;
    }
    void setHp(float hp) {
        currentHp_ = hp;
    }
    void Damage(float damage) {
        currentHp_ -= damage;
    }

    float getSpeed() const {
        return speed_;
    }
    void setSpeed(float speed) {
        currentSpeed_ = speed;
    }

    float getAttack() const {
        return attack_;
    }

    void setInvisibleTime(float time) {
        isInvisible_   = true;
        invisibleTime_ = time;
    }
    bool getIsInvisible() const {
        return isInvisible_;
    }

    AttackCollider* getAttackCollider() const { return attackCollider_.get(); }
    void setAttackCollider(std::unique_ptr<AttackCollider>& attackCollider) {
        attackCollider_ = std::move(attackCollider);
    }
    void resetAttackCollider() {
        if (attackCollider_) {
            attackCollider_->setIsAlive(false);
        }
    }
};
