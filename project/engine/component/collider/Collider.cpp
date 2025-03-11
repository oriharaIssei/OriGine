#include "Collider.h"

ICollider::ICollider(GameEntity* _hostEntity) : IComponent(_hostEntity) {
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
