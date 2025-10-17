#include "OBBCollider.h"

void to_json(nlohmann::json& _json, const OBBCollider& _o) {
    _json["center"]       = _o.getLocalCenter();
    _json["halfSize"]     = _o.getLocalHalfSize();
    _json["orientations"] = _o.shape_.orientations_.rot;
    _json["transform"]    = _o.transform_;
}
void from_json(const nlohmann::json& _json, OBBCollider& _o) {
    _json.at("center").get_to(_o.shape_.center_);
    _json.at("halfSize").get_to(_o.shape_.halfSize_);
    _json.at("orientations").get_to(_o.shape_.orientations_.rot);
    _o.shape_.orientations_.UpdateAxes();
    _json.at("transform").get_to(_o.transform_);
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
