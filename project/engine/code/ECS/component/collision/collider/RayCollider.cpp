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

Bounds::AABB RayCollider::ToWorldAABB() const {
    // Rayは無限なので、実用的な範囲でAABBを計算
    const float kMaxDistance = 1000.0f;
    Vec3f endPoint           = worldShape_.origin + worldShape_.direction * kMaxDistance;

    Vec3f minPt, maxPt;
    for (int i = 0; i < 3; ++i) {
        minPt[i] = std::min(worldShape_.origin[i], endPoint[i]);
        maxPt[i] = std::max(worldShape_.origin[i], endPoint[i]);
    }

    Vec3f center   = (minPt + maxPt) * 0.5f;
    Vec3f halfSize = (maxPt - minPt) * 0.5f;

    const float kMinSize = 0.001f;
    for (int i = 0; i < 3; ++i) {
        if (halfSize[i] < kMinSize)
            halfSize[i] = kMinSize;
    }

    return Bounds::AABB(center, halfSize);
}

} // namespace OriGine
