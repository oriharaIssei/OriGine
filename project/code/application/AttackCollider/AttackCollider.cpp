#include "AttackCollider.h"

//component
#include "application/Collision/Collider.h"

AttackCollider::AttackCollider(const std::string& id)
    : GameObject(id),
      id_(id) {}

AttackCollider::~AttackCollider() {
}

void AttackCollider::Init() {
    isAlive_     = true;
    hitCollider_ = std::make_unique<Collider>(id_);
}

void AttackCollider::Update() {
    hitCollider_->UpdateMatrix();
}

void AttackCollider::resetRadius(const std::string& id) {
    id_ = id;
    hitCollider_->resetRadius(id);
}

void AttackCollider::ColliderInit(
    const Vec3f& position,
    std::function<void(GameObject*)> onCollision) {
    hitCollider_->Init(onCollision);
    hitCollider_->setHostObject(this);
    hitCollider_->setTranslate(position);

    hitCollider_->UpdateMatrix();
}
