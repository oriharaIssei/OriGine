#include "SpotLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

bool SpotLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= CheckBoxCommand("Active", isActive_);

    ImGui::Spacing();

    isChanged |= ColorEditCommand("Color", color_);
    isChanged |= DragCommand<float>("Intensity", intensity_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= DragVectorCommand<3, float>("Position", pos_, 0.01f);
    isChanged |= DragVectorCommand<3, float>("Direction", direction_, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f::Normalize(*_d); });

    isChanged |= DragCommand<float>("Distance", distance_, 0.01f, 0.0f, 10.0f);
    isChanged |= DragCommand<float>("Decay", decay_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= DragCommand<float>("CosAngle", cosAngle_, 0.01f, 0.0f, 1.0f);
    isChanged |= DragCommand<float>("CosFalloffStart", cosFalloffStart_, 0.01f, 0.0f, 1.0f);

    return isChanged;
#else
    return false;
#endif // _DEBUG
}

void to_json(nlohmann::json& j, const SpotLight& l) {
    j["isActive"]        = l.isActive_;
    j["color"]           = l.color_;
    j["intensity"]       = l.intensity_;
    j["pos"]             = l.pos_;
    j["distance"]        = l.distance_;
    j["direction"]       = l.direction_;
    j["decay"]           = l.decay_;
    j["cosAngle"]        = l.cosAngle_;
    j["cosFalloffStart"] = l.cosFalloffStart_;
}

void from_json(const nlohmann::json& j, SpotLight& l) {
    j.at("isActive").get_to(l.isActive_);
    j.at("color").get_to(l.color_);
    j.at("intensity").get_to(l.intensity_);
    j.at("pos").get_to(l.pos_);
    j.at("distance").get_to(l.distance_);
    j.at("direction").get_to(l.direction_);
    j.at("decay").get_to(l.decay_);
    j.at("cosAngle").get_to(l.cosAngle_);
    j.at("cosFalloffStart").get_to(l.cosFalloffStart_);
}
