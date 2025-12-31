#include "PointLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void PointLight::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiCommand<float>("Radius##" + _parentLabel, radius, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const PointLight& l) {
    j["isActive"]  = l.isActive;
    j["color"]     = l.color;
    j["intensity"] = l.intensity;
    j["pos"]       = l.pos;
    j["radius"]    = l.radius;
    j["decay"]     = l.decay;
}
void OriGine::from_json(const nlohmann::json& j, PointLight& l) {
    j.at("isActive").get_to(l.isActive);
    j.at("color").get_to(l.color);
    j.at("intensity").get_to(l.intensity);
    j.at("pos").get_to(l.pos);
    j.at("radius").get_to(l.radius);
    j.at("decay").get_to(l.decay);
}
