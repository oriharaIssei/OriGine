#include "Rigidbody.h"

/// externals
#include "imgui/imgui.h"

Rigidbody::Rigidbody() {}

void Rigidbody::Initialize(GameEntity* _entity) {
    _entity;
}

bool Rigidbody::Edit() {
    bool isChange = false;

    isChange |= ImGui::InputFloat3("acceleration", acceleration_.v);
    isChange |= ImGui::InputFloat3("velocity", velocity_.v);

    ImGui::Separator();

    isChange |= ImGui::InputFloat("mass", &mass_);
    isChange |= ImGui::Checkbox("useGravity", &useGravity_);
    return isChange;
}

void Rigidbody::Finalize() {}

void to_json(nlohmann::json& j, const Rigidbody& r) {
    j["acceleration"] = r.acceleration_;
    j["velocity"]     = r.velocity_;
    j["mass"]         = r.mass_;
    j["useGravity"]   = r.useGravity_;
}
void from_json(const nlohmann::json& j, Rigidbody& r) {
    j.at("acceleration").get_to(r.acceleration_);
    j.at("velocity").get_to(r.velocity_);
    j.at("mass").get_to(r.mass_);
    j.at("useGravity").get_to(r.useGravity_);
}
