#include "Collider.h"

Collider::Collider(const std::string& name)
    : radius_("Game", name, "radius") {}

Collider::~Collider() {}

void Collider::Init(std::function<void(GameObject*)> _onCollision) {
    transform_.Init();

    currentRadius_ = radius_;

    onCollision_ = _onCollision;

    isAlive_ = true;
}

void Collider::OnCollision(GameObject* collisionObject) {
    if (onCollision_) {
        onCollision_(collisionObject);
    }
}
