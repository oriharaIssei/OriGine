#include "SquashStretchComponent.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SquashStretchComponent::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {}
void SquashStretchComponent::Finalize() {}

void SquashStretchComponent::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    DragGuiVectorCommand("Base Scale##" + _parentLabel, baseScale_, 0.01f);

    ImGui::Spacing();

    DragGuiCommand("Velocity Stretch ThresholdEnter##" + _parentLabel, velocityStretchThresholdEnter_, 0.01f);
    DragGuiCommand("Velocity Stretch ThresholdExit##" + _parentLabel, velocityStretchThresholdExit_, 0.01f);
    DragGuiCommand("Accel Squash ThresholdEnter##" + _parentLabel, accelSquashThresholdEnter_, 0.01f);
    DragGuiCommand("Accel Squash ThresholdExit##" + _parentLabel, accelSquashThresholdExit_, 0.01f);

    ImGui::Spacing();

    DragGuiCommand("Velocity Stretch Influence##" + _parentLabel, velocityStretchInfluence_, 0.01f);
    DragGuiCommand("Accel Squash Influence##" + _parentLabel, accelSquashInfluence_, 0.01f);

    ImGui::Spacing();

    DragGuiVectorCommand("Stiffness##" + _parentLabel, stiffness_, 0.01f);
    DragGuiVectorCommand("Damping##" + _parentLabel, damping_, 0.01f);

    ImGui::Spacing();

    DragGuiCommand("Max Stretch##" + _parentLabel, maxStretch_, 0.01f);
    DragGuiCommand("Max Squash##" + _parentLabel, maxSquash_, 0.01f);
#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const SquashStretchComponent& _component) {
    _j = nlohmann::json{
        {"baseScale", _component.baseScale_},
        {"scaleVelocity", _component.scaleVelocity_},
        {"preVelocity", _component.preVelocity_},
        {"velocityStretchThresholdEnter", _component.velocityStretchThresholdEnter_},
        {"velocityStretchThresholdExit", _component.velocityStretchThresholdExit_},
        {"accelSquashThresholdEnter", _component.accelSquashThresholdEnter_},
        {"accelSquashThresholdExit", _component.accelSquashThresholdExit_},
        {"velocityStretchInfluence", _component.velocityStretchInfluence_},
        {"accelSquashInfluence", _component.accelSquashInfluence_},
        {"stiffness", _component.stiffness_},
        {"damping", _component.damping_},
        {"maxStretch", _component.maxStretch_},
        {"maxSquash", _component.maxSquash_},
    };
}

void OriGine::from_json(const nlohmann::json& _j, SquashStretchComponent& _component) {
    _j.at("baseScale").get_to(_component.baseScale_);
    _j.at("velocityStretchThresholdEnter").get_to(_component.velocityStretchThresholdEnter_);
    _j.at("velocityStretchThresholdExit").get_to(_component.velocityStretchThresholdExit_);
    _j.at("accelSquashThresholdEnter").get_to(_component.accelSquashThresholdEnter_);
    _j.at("accelSquashThresholdExit").get_to(_component.accelSquashThresholdExit_);
    _j.at("velocityStretchInfluence").get_to(_component.velocityStretchInfluence_);
    _j.at("accelSquashInfluence").get_to(_component.accelSquashInfluence_);
    _j.at("stiffness").get_to(_component.stiffness_);
    _j.at("damping").get_to(_component.damping_);
    _j.at("maxStretch").get_to(_component.maxStretch_);
    _j.at("maxSquash").get_to(_component.maxSquash_);
}
