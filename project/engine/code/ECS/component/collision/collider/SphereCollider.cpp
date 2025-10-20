#include "SphereCollider.h"

void to_json(nlohmann::json& _json, const SphereCollider& _s) {
    _json["center"]    = _s.getLocalCenter();
    _json["radius"]    = _s.getLocalRadius();
    _json["transform"] = _s.transform_;
}
void from_json(const nlohmann::json& _json, SphereCollider& _s) {
    _json.at("center").get_to(_s.shape_.center_);
    _json.at("radius").get_to(_s.shape_.radius_);
    _json.at("transform").get_to(_s.transform_);
}

void SphereCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {

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
