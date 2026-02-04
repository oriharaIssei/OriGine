#include "OBBCollider.h"

using namespace OriGine;

void OriGine::to_json(nlohmann::json& _json, const OBBCollider& _o) {
    to_json(_json, static_cast<const ICollider&>(_o));
    _json["center"]       = _o.GetLocalCenter();
    _json["halfSize"]     = _o.GetLocalHalfSize();
    _json["orientations"] = _o.shape_.orientations_.rot;
    _json["transform"]    = _o.transform_;
}
void OriGine::from_json(const nlohmann::json& _json, OBBCollider& _o) {
    from_json(_json, static_cast<ICollider&>(_o));
    if (_json.contains("center")) {
        _json.at("center").get_to(_o.shape_.center_);
    }
    if (_json.contains("halfSize")) {
        _json.at("halfSize").get_to(_o.shape_.halfSize_);
    }
    if (_json.contains("orientations")) {
        _json.at("orientations").get_to(_o.shape_.orientations_.rot);
        _o.shape_.orientations_.UpdateAxes();
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_o.transform_);
    }
}

void OBBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    ICollider::Edit(_scene, _handle, _parentLabel);

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
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif
};

void OBBCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.center_           = transform_.GetWorldTranslate() + shape_.center_;
    this->worldShape_.halfSize_         = shape_.halfSize_ * transform_.GetWorldScale();
    this->worldShape_.orientations_.rot = shape_.orientations_.rot * transform_.CalculateWorldRotate();
    this->worldShape_.orientations_.UpdateAxes();
}

Bounds::AABB OBBCollider::ToWorldAABB() const {
    // OBBの各軸に沿った投影からAABBを計算
    const auto& axes = worldShape_.orientations_.axis;
    const auto& half = worldShape_.halfSize_;

    Vec3f extent;
    for (int i = 0; i < 3; ++i) {
        extent[i] = std::abs(axes[X][i]) * half[X] + std::abs(axes[Y][i]) * half[Y] + std::abs(axes[Z][i]) * half[Z];
    }

    return Bounds::AABB(worldShape_.center_, extent);
}
