#include "SpotLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

bool SpotLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    
    isChanged |= ImGui::Checkbox("Active", &isActive_);

    ImGui::Spacing();

    isChanged |= ImGui::ColorEdit3("Color", color_.v);
    isChanged |= ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= ImGui::DragFloat3("Position", pos_.v, 0.01f);
    if (ImGui::DragFloat3("Direction", direction_.v, 0.01f)) {
        direction_ = direction_.normalize();
        isChanged  = true;
    }
    isChanged |= ImGui::DragFloat("Distance", &distance_, 0.01f, 0.0f, 10.0f);
    isChanged |= ImGui::DragFloat("Decay", &decay_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= ImGui::DragFloat("CosAngle", &cosAngle_, 0.01f, 0.0f, 1.0f);
    isChanged |= ImGui::DragFloat("CosFalloffStart", &cosFalloffStart_, 0.01f, 0.0f, 1.0f);

    return isChanged;
#else
    return false;
#endif // _DEBUG
}
