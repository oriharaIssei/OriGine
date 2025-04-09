#include "Collider.h"

void ICollider::Initialize(GameEntity* _hostEntity) {
    this->transform_.parent = getComponent<Transform>(_hostEntity);
}

void ICollider::StartCollision() {
    this->preCollisionStateMap_ = this->collisionStateMap_;
    this->collisionStateMap_.clear();
}

void ICollider::EndCollision() {
    for (auto& [entity, state] : this->preCollisionStateMap_) {
        if (state == CollisionState::Exit)
            return;
        if (this->collisionStateMap_[entity] == CollisionState::None)
            this->collisionStateMap_[entity] = CollisionState::Exit;
    }
}

void AABBCollider::CalculateWorldShape() {
    transform_.Update();
    this->worldShape_.min_ = Vec3f(transform_.worldMat[3]) + shape_.min_;
    this->worldShape_.max_ = Vec3f(transform_.worldMat[3]) + shape_.max_;
}

void SphereCollider::CalculateWorldShape() {
    transform_.Update();
    this->worldShape_.center_ = Vec3f(transform_.worldMat[3]) + shape_.center_;
    this->worldShape_.radius_ = shape_.radius_;
}
