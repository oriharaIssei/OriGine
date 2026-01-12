#include "Rigidbody.h"

/// externals
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"

#include "util/deltaTime/DeltaTimer.h"
#endif // _DEBUG

using namespace OriGine;

Rigidbody::Rigidbody() {}

void Rigidbody::Initialize(Scene* /*_scene*/, EntityHandle /*_handle*/) {}

void Rigidbody::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    DragGuiVectorCommand("acceleration##" + _parentLabel, acceleration_);
    DragGuiVectorCommand("velocity##" + _parentLabel, velocity_);
    DragGuiCommand("maxXZSpeed##" + _parentLabel, maxXZSpeed_);

    ImGui::Separator();

    CheckBoxCommand("useGravity##" + _parentLabel, useGravity_);
    DragGuiCommand("mass##" + _parentLabel, mass_);
    DragGuiCommand("maxFallSpeed##" + _parentLabel, maxFallSpeed_, 0.1f, 0.f, 10000.f, "%.3f");

    ImGui::Separator();

    CheckBoxCommand("isUsingLocalDeltaTime##" + _parentLabel, isUsingLocalDeltaTime_);
    std::string label = "localDeltaTimeName##" + _parentLabel;
    if (isUsingLocalDeltaTime_) {
        ImGui::InputText(label.c_str(), &localDeltaTimeName_);
    } else {
        localDeltaTimeName_ = "";
    }

#endif // _DEBUG
}

void Rigidbody::Debug() {
#ifdef _DEBUG
    ImGui::DragFloat3("acceleration", acceleration_.v, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
    ImGui::Text("acceleration Speed : %.3f", acceleration_.length());
    ImGui::DragFloat3("velocity", velocity_.v, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
    ImGui::Text("velocity Speed : %.3f", velocity_.length());
    ImGui::Separator();
    ImGui::Checkbox("Use Gravity", &useGravity_);
    ImGui::DragFloat("Mass", &mass_, 0.1f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_ReadOnly);
#endif // _DEBUG
}

void Rigidbody::Finalize() {}

void OriGine::to_json(nlohmann::json& j, const Rigidbody& r) {
    j["acceleration"] = r.acceleration_;
    j["velocity"]     = r.velocity_;
    j["maxXZSpeed"]   = r.maxXZSpeed_;
    j["mass"]         = r.mass_;
    j["useGravity"]   = r.useGravity_;
    j["maxFallSpeed"] = r.maxFallSpeed_;

    j["isUsingLocalDeltaTime"] = r.isUsingLocalDeltaTime_;
    j["localDeltaTimeName"]    = r.localDeltaTimeName_;
}
void OriGine::from_json(const nlohmann::json& j, Rigidbody& r) {
    j.at("acceleration").get_to(r.acceleration_);
    j.at("velocity").get_to(r.velocity_);
    if (j.contains("maxXZSpeed")) {
        j.at("maxXZSpeed").get_to(r.maxXZSpeed_);
    }

    j.at("mass").get_to(r.mass_);
    j.at("useGravity").get_to(r.useGravity_);
    if (j.contains("maxFallSpeed")) {
        j.at("maxFallSpeed").get_to(r.maxFallSpeed_);
    }

    if (j.contains("isUsingLocalDeltaTime")) {
        j.at("isUsingLocalDeltaTime").get_to(r.isUsingLocalDeltaTime_);
    }
    if (j.contains("localDeltaTimeName")) {
        j.at("localDeltaTimeName").get_to(r.localDeltaTimeName_);
    }
}
