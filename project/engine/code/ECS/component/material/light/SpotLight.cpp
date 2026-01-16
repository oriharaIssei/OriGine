#include "SpotLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SpotLight::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f::Normalize(*_d); });
    direction = Vec3f::Normalize(direction);

    DragGuiCommand<float>("Distance##" + _parentLabel, distance, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiCommand<float>("CosAngle##" + _parentLabel, cosAngle, 0.01f, 0.0f, 1.0f);
    DragGuiCommand<float>("CosFalloffStart##" + _parentLabel, cosFalloffStart, 0.01f, 0.0f, 1.0f);
    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.01f, 0.0f);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const SpotLight& _comp) {
    _j["isActive"]        = _comp.isActive;
    _j["color"]           = _comp.color;
    _j["intensity"]       = _comp.intensity;
    _j["pos"]             = _comp.pos;
    _j["distance"]        = _comp.distance;
    _j["direction"]       = _comp.direction;
    _j["decay"]           = _comp.decay;
    _j["cosAngle"]        = _comp.cosAngle;
    _j["cosFalloffStart"] = _comp.cosFalloffStart;
    _j["angularRadius"]   = _comp.angularRadius;
}

void OriGine::from_json(const nlohmann::json& _j, SpotLight& _comp) {
    _j.at("isActive").get_to(_comp.isActive);
    _j.at("color").get_to(_comp.color);
    _j.at("intensity").get_to(_comp.intensity);
    _j.at("pos").get_to(_comp.pos);
    _j.at("distance").get_to(_comp.distance);
    _j.at("direction").get_to(_comp.direction);
    _j.at("decay").get_to(_comp.decay);
    _j.at("cosAngle").get_to(_comp.cosAngle);
    _j.at("cosFalloffStart").get_to(_comp.cosFalloffStart);
    if (_j.contains("angularRadius")) {
        _j.at("angularRadius").get_to(_comp.angularRadius);
    }
}
