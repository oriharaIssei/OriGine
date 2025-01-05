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

    std::unique_ptr<Collider> hitCollider_ = nullptr;

public:
    void ColliderInit(
        const Vector3& position,
        std::function<void(GameObject*)> onCollision);

    Collider* getHitCollider() const { return hitCollider_.get(); }
};
