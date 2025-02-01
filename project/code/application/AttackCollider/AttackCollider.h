#pragma once

///stl
//pointer
#include <functional>
#include <memory>
//string
#include <string>

///engine
//component
class Collider;
//object
#include "../GameObject/GameObject.h"

class AttackCollider
    : public GameObject {
public:
    AttackCollider(const std::string& id);
    ~AttackCollider();

    void Init();
    void Update();

private:
    std::string id_;

public:
    void resetRadius(const std::string& id);
    void ColliderInit(
        const Vec3f& position,
        std::function<void(GameObject*)> onCollision);

    void SetHitColliderPos(const Vec3f& _pos) {
        hitCollider_->setTranslate(_pos);
    }
};
