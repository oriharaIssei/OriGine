#include "SpotLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SpotLight::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f::Normalize(*_d); });

    DragGuiCommand<float>("Distance##" + _parentLabel, distance, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiCommand<float>("CosAngle##" + _parentLabel, cosAngle, 0.01f, 0.0f, 1.0f);
    DragGuiCommand<float>("CosFalloffStart##" + _parentLabel, cosFalloffStart, 0.01f, 0.0f, 1.0f);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const SpotLight& l) {
    j["isActive"]        = l.isActive;
    j["color"]           = l.color;
    j["intensity"]       = l.intensity;
    j["pos"]             = l.pos;
    j["distance"]        = l.distance;
    j["direction"]       = l.direction;
    j["decay"]           = l.decay;
    j["cosAngle"]        = l.cosAngle;
    j["cosFalloffStart"] = l.cosFalloffStart;
}

void OriGine::from_json(const nlohmann::json& j, SpotLight& l) {
    j.at("isActive").get_to(l.isActive);
    j.at("color").get_to(l.color);
    j.at("intensity").get_to(l.intensity);
    j.at("pos").get_to(l.pos);
    j.at("distance").get_to(l.distance);
    j.at("direction").get_to(l.direction);
    j.at("decay").get_to(l.decay);
    j.at("cosAngle").get_to(l.cosAngle);
    j.at("cosFalloffStart").get_to(l.cosFalloffStart);
}
