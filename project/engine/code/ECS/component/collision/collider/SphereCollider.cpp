#include "SphereCollider.h"

using namespace OriGine;

void OriGine::to_json(nlohmann::json& _json, const SphereCollider& _s) {
    to_json(_json, static_cast<const ICollider&>(_s));
    _json["center"]    = _s.GetLocalCenter();
    _json["radius"]    = _s.GetLocalRadius();
    _json["transform"] = _s.transform_;
}
void OriGine::from_json(const nlohmann::json& _json, SphereCollider& _s) {
    from_json(_json, static_cast<ICollider&>(_s));
    if (_json.contains("center")) {
        _json.at("center").get_to(_s.shape_.center_);
    }
    if (_json.contains("radius")) {
        _json.at("radius").get_to(_s.shape_.radius_);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_s.transform_);
    }
}

void SphereCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "Sphere##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, shape_.center_, 0.01f);
        DragGuiCommand<float>("Radius##" + _parentLabel, shape_.radius_, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void SphereCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.center_ = Vec3f(transform_.worldMat[3]) + shape_.center_;
    this->worldShape_.radius_ = shape_.radius_;
}

Bounds::AABB SphereCollider::ToWorldAABB() const {
    Vec3f halfSize(worldShape_.radius_, worldShape_.radius_, worldShape_.radius_);
    return Bounds::AABB(worldShape_.center_, halfSize);
}
