#include "Collider.h"

void ICollider::Initialize(GameEntity* /*_hostEntity*/) {}

void ICollider::StartCollision() {
    this->preCollisionStateMap_ = this->collisionStateMap_;
    this->collisionStateMap_.clear();

    CalculateWorldShape();
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

void to_json(nlohmann::json& _json, const AABBCollider& _primitiveNodeAnimation) {
    _json["min"]       = _primitiveNodeAnimation.getLocalMin();
    _json["max"]       = _primitiveNodeAnimation.getLocalMax();
    _json["transform"] = _primitiveNodeAnimation.transform_;
}
void from_json(const nlohmann::json& _json, AABBCollider& _primitiveNodeAnimation) {
    _json.at("min").get_to(_primitiveNodeAnimation.shape_.min_);
    _json.at("max").get_to(_primitiveNodeAnimation.shape_.max_);
    _json.at("transform").get_to(_primitiveNodeAnimation.transform_);
}
void to_json(nlohmann::json& _json, const SphereCollider& _primitiveNodeAnimation) {
    _json["center"]    = _primitiveNodeAnimation.getLocalCenter();
    _json["radius"]    = _primitiveNodeAnimation.getLocalRadius();
    _json["transform"] = _primitiveNodeAnimation.transform_;
}
void from_json(const nlohmann::json& _json, SphereCollider& _primitiveNodeAnimation) {
    _json.at("center").get_to(_primitiveNodeAnimation.shape_.center_);
    _json.at("radius").get_to(_primitiveNodeAnimation.shape_.radius_);
    _json.at("transform").get_to(_primitiveNodeAnimation.transform_);
}
