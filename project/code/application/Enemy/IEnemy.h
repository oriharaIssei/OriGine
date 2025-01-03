#pragma once

/// stl
#include <memory>
#include <string>

/// engine
#include "../GameObject/GameObject.h"
#include "globalVariables/SerializedField.h"
#include "transform/Transform.h"

/// application
//Behavior
#include "BehaviorTree/DefaultNodes.h"

//math
#include "Quaternion.h"
#include "Vector3.h"

class Object3d;
class Player;

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

    std::unique_ptr<EnemyBehavior::Node> behaviorTree_ = nullptr;

    SerializedField<float> hp_;
    SerializedField<float> speed_;
    SerializedField<float> attack_;

    float currentHp_             = 0.0f;
    float currentSpeed_          = 0.0f;
    float currentAttack_         = 0.0f;

public:
    Player* getPlayer() { return player_; }
    void setPlayer(Player* player) {
        player_ = player;
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
    void setAttack(float attack) {
        currentAttack_ = attack;
    }
};
