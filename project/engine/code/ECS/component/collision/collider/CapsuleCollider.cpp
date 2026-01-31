#include "CapsuleCollider.h"

namespace OriGine {

void to_json(nlohmann::json& _json, const CapsuleCollider& _c) {
    _json["start"]     = _c.shape_.segment.start;
    _json["end"]       = _c.shape_.segment.end;
    _json["radius"]    = _c.shape_.radius;
    _json["transform"] = _c.transform_;
}

void from_json(const nlohmann::json& _json, CapsuleCollider& _c) {
    if (_json.contains("start")) {
        _json.at("start").get_to(_c.shape_.segment.start);
    }
    if (_json.contains("end")) {
        _json.at("end").get_to(_c.shape_.segment.end);
    }
    if (_json.contains("radius")) {
        _json.at("radius").get_to(_c.shape_.radius);
    }
    _json.at("transform").get_to(_c.transform_);
}

void CapsuleCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "Capsule##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Start##" + _parentLabel, this->shape_.segment.start, 0.01f);
        DragGuiVectorCommand<3, float>("End##" + _parentLabel, this->shape_.segment.end, 0.01f);
        DragGuiCommand<float>("Radius##" + _parentLabel, this->shape_.radius, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void CapsuleCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.segment.start = shape_.segment.start * transform_.worldMat;
    this->worldShape_.segment.end   = shape_.segment.end * transform_.worldMat;
    // スケールの最大値を半径に適用
    Vec3f scale              = transform_.GetWorldScale();
    float maxScale           = std::max({scale[X], scale[Y], scale[Z]});
    this->worldShape_.radius = shape_.radius * maxScale;
}

} // namespace OriGine
