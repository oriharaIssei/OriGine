#include "Rigidbody.h"

/// externals
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

Rigidbody::Rigidbody() {}

void Rigidbody::Initialize(GameEntity* _entity) {
    _entity;
}

bool Rigidbody::Edit() {
    bool isChange = false;

#ifdef _DEBUG

    DragGuiVectorCommand("acceleration", acceleration_);
    DragGuiVectorCommand("velocity", velocity_);

    ImGui::Separator();

    CheckBoxCommand("useGravity", useGravity_);
    DragGuiCommand("mass", mass_);
    DragGuiCommand("maxFallSpeed", maxFallSpeed_, 0.1f, 0.f, 10000.f, "%.3f");

#endif // _DEBUG

    return isChange;
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

void to_json(nlohmann::json& j, const Rigidbody& r) {
    j["acceleration"] = r.acceleration_;
    j["velocity"]     = r.velocity_;
    j["mass"]         = r.mass_;
    j["useGravity"]   = r.useGravity_;
    j["maxFallSpeed"] = r.maxFallSpeed_;
}
void from_json(const nlohmann::json& j, Rigidbody& r) {
    j.at("acceleration").get_to(r.acceleration_);
    j.at("velocity").get_to(r.velocity_);
    j.at("mass").get_to(r.mass_);
    j.at("useGravity").get_to(r.useGravity_);
    if (j.contains("maxFallSpeed")) {
        j.at("maxFallSpeed").get_to(r.maxFallSpeed_);
    }
}
