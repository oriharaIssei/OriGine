#include "PointLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

bool PointLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= ImGui::Checkbox("Active", &isActive_);

    ImGui::Spacing();

    isChanged |= ImGui::ColorEdit3("Color", color_.v);
    isChanged |= ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= ImGui::DragFloat3("Position", pos_.v, 0.01f);
    isChanged |= ImGui::DragFloat("Radius", &radius_, 0.01f, 0.0f, 10.0f);
    isChanged |= ImGui::DragFloat("Decay", &decay_, 0.01f, 0.0f, 10.0f);
    return isChanged;
#else
    return false;
#endif // _DEBUG
}

void to_json(nlohmann::json& j, const PointLight& l) {
    j["isActive"]  = l.isActive_;
    j["color"]     = l.color_;
    j["intensity"] = l.intensity_;
    j["pos"]       = l.pos_;
    j["radius"]    = l.radius_;
    j["decay"]     = l.decay_;
}
void from_json(const nlohmann::json& j, PointLight& l) {
    j.at("isActive").get_to(l.isActive_);
    j.at("color").get_to(l.color_);
    j.at("intensity").get_to(l.intensity_);
    j.at("pos").get_to(l.pos_);
    j.at("radius").get_to(l.radius_);
    j.at("decay").get_to(l.decay_);
}
