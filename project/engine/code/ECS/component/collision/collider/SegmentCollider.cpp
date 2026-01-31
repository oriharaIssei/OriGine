#include "SegmentCollider.h"

namespace OriGine {

void to_json(nlohmann::json& _json, const SegmentCollider& _s) {
    _json["start"]     = _s.shape_.start;
    _json["end"]       = _s.shape_.end;
    _json["transform"] = _s.transform_;
}

void from_json(const nlohmann::json& _json, SegmentCollider& _s) {
    if (_json.contains("start")) {
        _json.at("start").get_to(_s.shape_.start);
    }
    if (_json.contains("end")) {
        _json.at("end").get_to(_s.shape_.end);
    }
    _json.at("transform").get_to(_s.transform_);
}

void SegmentCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive", this->isActive_);

    std::string label = "Segment##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Start##" + _parentLabel, this->shape_.start, 0.01f);
        DragGuiVectorCommand<3, float>("End##" + _parentLabel, this->shape_.end, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void SegmentCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    this->worldShape_.start = shape_.start * transform_.worldMat;
    this->worldShape_.end   = shape_.end * transform_.worldMat;
}

} // namespace OriGine
