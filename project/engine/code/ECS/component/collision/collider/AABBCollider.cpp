#include "AABBCollider.h"

void to_json(nlohmann::json& _json, const AABBCollider& _a) {
    _json["center"]    = _a.shape_.center;
    _json["halfSize"]  = _a.shape_.halfSize;
    _json["transform"] = _a.transform_;
}
void from_json(const nlohmann::json& _json, AABBCollider& _a) {
    if (_json.contains("center")) {
        _json.at("center").get_to(_a.shape_.center);
    }
    if (_json.contains("halfSize")) {
        _json.at("halfSize").get_to(_a.shape_.halfSize);
    }
    _json.at("transform").get_to(_a.transform_);
}

void AABBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "AABB##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, this->shape_.center, 0.01f);
        DragGuiVectorCommand<3, float>("HalfSize##" + _parentLabel, this->shape_.halfSize, 0.01f);
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
    this->worldShape_.center   = shape_.center * transform_.worldMat;
    this->worldShape_.halfSize = shape_.halfSize * transform_.GetWorldScale();
}
