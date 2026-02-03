#include "RayCollider.h"

namespace OriGine {

void to_json(nlohmann::json& _json, const RayCollider& _r) {
    to_json(_json, static_cast<const ICollider&>(_r));
    _json["origin"]    = _r.shape_.origin;
    _json["direction"] = _r.shape_.direction;
    _json["transform"] = _r.transform_;
}

void from_json(const nlohmann::json& _json, RayCollider& _r) {
    from_json(_json, static_cast<ICollider&>(_r));
    if (_json.contains("origin")) {
        _json.at("origin").get_to(_r.shape_.origin);
    }
    if (_json.contains("direction")) {
        _json.at("direction").get_to(_r.shape_.direction);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_r.transform_);
    }
}

void RayCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "Ray##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Origin##" + _parentLabel, this->shape_.origin, 0.01f);
        DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, this->shape_.direction, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void RayCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.origin    = shape_.origin * transform_.worldMat;
    this->worldShape_.direction = (shape_.direction * MakeMatrix4x4::RotateQuaternion(transform_.CalculateWorldRotate())).normalize();
}

} // namespace OriGine
