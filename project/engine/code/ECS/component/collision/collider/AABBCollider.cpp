#include "AABBCollider.h"

void to_json(nlohmann::json& _json, const AABBCollider& _a) {
    _json["min"]       = _a.GetLocalMin();
    _json["max"]       = _a.GetLocalMax();
    _json["transform"] = _a.transform_;
}
void from_json(const nlohmann::json& _json, AABBCollider& _a) {
    _json.at("min").get_to(_a.shape_.min_);
    _json.at("max").get_to(_a.shape_.max_);
    _json.at("transform").get_to(_a.transform_);
}

void AABBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
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
