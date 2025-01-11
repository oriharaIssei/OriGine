#pragma once

//parent
#include "DefaultNodes.h"

///stl
//pointer
#include <functional>
//string
#include <string>

///object
class GameObject;
//lib
#include "globalVariables/SerializedField.h"


namespace EnemyBehavior {
class CreateAttackCollider
    : public Action {
public:
    CreateAttackCollider(
        const std::string& _colliderID,
        const Vec3f& _colliderOffset,
        std::function<void(GameObject*)> onCollision = nullptr);
    ~CreateAttackCollider();

    Status tick() override;

private:
    std::string colliderID_;
    Vec3f colliderOffset_;
    std::function<void(GameObject*)> onCollision_ = nullptr;
};

class WeakAttackAction
    : public Action {
public:
    WeakAttackAction(float _attack);
    ~WeakAttackAction();

    Status tick() override;

private:
    float attackPower_ = 0.0f;
    bool isEndMotion_  = false;
};
} // namespace EnemyBehavior
