#include "PointLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void PointLight::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiCommand<float>("Radius##" + _parentLabel, radius, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);
    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.01f, 0.0f);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const PointLight& _comp) {
    _j["isActive"]      = _comp.isActive;
    _j["color"]         = _comp.color;
    _j["intensity"]     = _comp.intensity;
    _j["pos"]           = _comp.pos;
    _j["radius"]        = _comp.radius;
    _j["decay"]         = _comp.decay;
    _j["angularRadius"] = _comp.angularRadius;
}
void OriGine::from_json(const nlohmann::json& _j, PointLight& _comp) {
    _j.at("isActive").get_to(_comp.isActive);
    _j.at("color").get_to(_comp.color);
    _j.at("intensity").get_to(_comp.intensity);
    _j.at("pos").get_to(_comp.pos);
    _j.at("radius").get_to(_comp.radius);
    _j.at("decay").get_to(_comp.decay);
    if (_j.contains("angularRadius")) {
        _j.at("angularRadius").get_to(_comp.angularRadius);
    }
}
