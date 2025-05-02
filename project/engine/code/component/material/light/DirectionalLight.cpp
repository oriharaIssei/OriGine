#include "DirectionalLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

bool DirectionalLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= ImGui::Checkbox("Active", &isActive_);

    ImGui::Spacing();

    isChanged |= ImGui::ColorEdit3("Color", color_.v);
    isChanged |= ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);
    if (ImGui::DragFloat3("Direction", direction_.v, 0.01f)) {
        direction_ = direction_.normalize();
        isChanged  = true;
    }

    return isChanged;
#else
    return false;
#endif // _DEBUG
}

void to_json(nlohmann::json& j, const DirectionalLight& l) {
    j["isActive"]  = l.isActive_;
    j["color"]     = l.color_;
    j["intensity"] = l.intensity_;
    j["direction"] = l.direction_;
}

void from_json(const nlohmann::json& j, DirectionalLight& l) {
    j.at("isActive").get_to(l.isActive_);
    j.at("color").get_to(l.color_);
    j.at("intensity").get_to(l.intensity_);
    j.at("direction").get_to(l.direction_);
}
