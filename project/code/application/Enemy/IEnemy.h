#pragma once

/// stl
#include <memory>
#include <string>

/// engine
#include "globalVariables/SerializedField.h"
#include "transform/Transform.h"

//math
#include "Quaternion.h"
#include "Vector3.h"

class Object3d;
class Player;

class IEnemy {
public:
    IEnemy(const std::string& _type);
    virtual ~IEnemy();

    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Draw();

    virtual std::unique_ptr<IEnemy> Clone() = 0;

protected:
    Player* player_ = nullptr;

    std::unique_ptr<Object3d> drawObject3d_;
    Transform transform_;

    SerializedField<float> hp_;
    SerializedField<float> speed_;
    SerializedField<float> attack_;
    SerializedField<float> maxAttackCoolTime_;
    SerializedField<float> minAttackCoolTime_;

    float currentHp_      = 0.0f;
    float currentSpeed_   = 0.0f;
    float currentAttack_  = 0.0f;
    float currentAttackCoolTime_ = 0.0f; // minAttackCoolTime_ ~ maxAttackCoolTime_

public:
    void setPlayer(Player* player) {
        player_ = player;
    }

    const Vector3& getScale() const {
        return transform_.scale;
    }
    const Quaternion& getRotate() const {
        return transform_.rotate;
    }
    const Vector3& getTranslate() const {
        return transform_.translate;
    }

    void setScale(const Vector3& s) {
        transform_.scale = s;
    }
    void setRotate(const Quaternion& q) {
        transform_.rotate = q;
    }
    void setTranslate(const Vector3& t) {
        transform_.translate = t;
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
