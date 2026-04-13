#include "DirectionalLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void DirectionalLight::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.1f);

    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f(*_d).normalize(); });
    direction = direction.normalize();

    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.0001f, {}, {}, "%.4f");

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const DirectionalLight& _comp) {
    _j["isActive"]      = _comp.isActive;
    _j["color"]         = _comp.color;
    _j["intensity"]     = _comp.intensity;
    _j["direction"]     = _comp.direction;
    _j["angularRadius"] = _comp.angularRadius;
}

void OriGine::from_json(const nlohmann::json& _j, DirectionalLight& _comp) {
    _j.at("isActive").get_to(_comp.isActive);
    _j.at("color").get_to(_comp.color);
    _j.at("intensity").get_to(_comp.intensity);
    _j.at("direction").get_to(_comp.direction);
    if (_j.contains("angularRadius")) {
        _j.at("angularRadius").get_to(_comp.angularRadius);
    }
}
