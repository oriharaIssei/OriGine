#include "Collider.h"

#include "../GameObject/GameObject.h"

Collider::Collider(const std::string& name)
    : radius_("Game", name, "radius") {}

Collider::~Collider() {}

void Collider::Init(std::function<void(GameObject*)> _onCollision) {
    isAlive_ = true;

    transform_.Init();

    currentRadius_ = radius_;

    onCollision_ = _onCollision;
}

void Collider::OnCollision(GameObject* collisionObject) {
    if (onCollision_) {
        onCollision_(collisionObject);
    }
}

void Collider::UpdateMatrix() {
    transform_.Update();

#ifdef _DEBUG
    currentRadius_ = radius_;
#endif // _DEBUG
}

void Collider::resetRadius(const std::string& id) {
    radius_        = {"Game", id, "radius"};
    currentRadius_ = radius_;
}
