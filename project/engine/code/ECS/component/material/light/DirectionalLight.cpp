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

    CheckBoxCommand("Active##" + _parentLabel, isActive_);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color_);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity_, 0.01f, 0.1f);

    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction_, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f(*_d).normalize(); });
    direction_ = direction_.normalize();
#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const DirectionalLight& l) {
    j["isActive"]  = l.isActive_;
    j["color"]     = l.color_;
    j["intensity"] = l.intensity_;
    j["direction"] = l.direction_;
}

void OriGine::from_json(const nlohmann::json& j, DirectionalLight& l) {
    j.at("isActive").get_to(l.isActive_);
    j.at("color").get_to(l.color_);
    j.at("intensity").get_to(l.intensity_);
    j.at("direction").get_to(l.direction_);
}
