#include "DirectionalLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

bool DirectionalLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= CheckBoxCommand("Active", isActive_);

    ImGui::Spacing();

    isChanged |= ColorEditGuiCommand("Color", color_);
    isChanged |= DragGuiCommand<float>("Intensity", intensity_, 0.01f, 0.1f);

    DragGuiVectorCommand<3, float>("Direction", direction_, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f(*_d).normalize(); });

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
