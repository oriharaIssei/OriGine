#include "DirectionalLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void DirectionalLight::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.1f);

    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f(*_d).normalize(); });
    direction = direction.normalize();

    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.0001f,{},{},"%.4f");

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const DirectionalLight& l) {
    j["isActive"]      = l.isActive;
    j["color"]         = l.color;
    j["intensity"]     = l.intensity;
    j["direction"]     = l.direction;
    j["angularRadius"] = l.angularRadius;
}

void OriGine::from_json(const nlohmann::json& j, DirectionalLight& l) {
    j.at("isActive").get_to(l.isActive);
    j.at("color").get_to(l.color);
    j.at("intensity").get_to(l.intensity);
    j.at("direction").get_to(l.direction);
    if (j.contains("angularRadius")) {
        j.at("angularRadius").get_to(l.angularRadius);
    }
}
