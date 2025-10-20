#include "PointLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void PointLight::Edit(Scene* /*_scene*/,Entity* /*_entity*/,[[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    
    CheckBoxCommand("Active##" + _parentLabel, isActive_);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color_);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos_, 0.01f);
    DragGuiCommand<float>("Radius##" + _parentLabel, radius_, 0.01f, 0.0f, 10.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay_, 0.01f, 0.0f, 10.0f);

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
