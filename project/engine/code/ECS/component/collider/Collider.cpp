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

void AABBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "AABB##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Min##" + _parentLabel, this->shape_.min_, 0.01f);
        DragGuiVectorCommand<3, float>("Max##" + _parentLabel, this->shape_.max_, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _entity, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void AABBCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.min_ = Vec3f(transform_.worldMat[3]) + shape_.min_;
    this->worldShape_.max_ = Vec3f(transform_.worldMat[3]) + shape_.max_;
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

void SphereCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "Sphere##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, shape_.center_, 0.01f);
        DragGuiCommand<float>("Radius##" + _parentLabel, shape_.radius_, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _entity, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void SphereCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.center_ = Vec3f(transform_.worldMat[3]) + shape_.center_;
    this->worldShape_.radius_ = shape_.radius_;
}

void to_json(nlohmann::json& _json, const OBBCollider& _primitiveNodeAnimation) {
    _json["center"]       = _primitiveNodeAnimation.getLocalCenter();
    _json["halfSize"]     = _primitiveNodeAnimation.getLocalHalfSize();
    _json["orientations"] = _primitiveNodeAnimation.shape_.orientations_.rot;
    _json["transform"]    = _primitiveNodeAnimation.transform_;
}
void from_json(const nlohmann::json& _json, OBBCollider& _primitiveNodeAnimation) {
    _json.at("center").get_to(_primitiveNodeAnimation.shape_.center_);
    _json.at("halfSize").get_to(_primitiveNodeAnimation.shape_.halfSize_);
    _json.at("orientations").get_to(_primitiveNodeAnimation.shape_.orientations_.rot);
    _primitiveNodeAnimation.shape_.orientations_.UpdateAxes();
    _json.at("transform").get_to(_primitiveNodeAnimation.transform_);
}

void OBBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "OBB##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, shape_.center_, 0.01f);
        DragGuiVectorCommand<3, float>("HalfSize##" + _parentLabel, shape_.halfSize_, 0.01f);

        ImGui::Spacing();

        DragGuiVectorCommand<4, float>("Rotation##" + _parentLabel, shape_.orientations_.rot, 0.01f);
        shape_.orientations_.UpdateAxes();

        ImGui::Text("Axes:");
        ImGui::Text("X: %.2f, %.2f, %.2f", shape_.orientations_.axis[X][X], shape_.orientations_.axis[X][Y], shape_.orientations_.axis[X][Z]);
        ImGui::Text("Y: %.2f, %.2f, %.2f", shape_.orientations_.axis[Y][X], shape_.orientations_.axis[Y][Y], shape_.orientations_.axis[Y][Z]);
        ImGui::Text("Z: %.2f, %.2f, %.2f", shape_.orientations_.axis[Z][X], shape_.orientations_.axis[Z][Y], shape_.orientations_.axis[Z][Z]);

        ImGui::TreePop();
    }

    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _entity, _parentLabel);
        ImGui::TreePop();
    }

#endif
};

void OBBCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.center_           = Vec3f(transform_.worldMat[3]) + shape_.center_;
    this->worldShape_.halfSize_         = shape_.halfSize_;
    this->worldShape_.orientations_.rot = shape_.orientations_.rot * transform_.CalculateWorldRotate();
    this->worldShape_.orientations_.UpdateAxes();
}
