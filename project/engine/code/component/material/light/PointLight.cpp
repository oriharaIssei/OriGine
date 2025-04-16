#include "PointLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

bool PointLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= ImGui::Checkbox("Active", &isActive_);

    ImGui::Spacing();

    isChanged |= ImGui::ColorEdit3("Color", color_.v);
    isChanged |= ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);

    ImGui::Spacing();

    isChanged |= ImGui::DragFloat3("Position", pos_.v, 0.01f);
    isChanged |= ImGui::DragFloat("Radius", &radius_, 0.01f, 0.0f, 10.0f);
    isChanged |= ImGui::DragFloat("Decay", &decay_, 0.01f, 0.0f, 10.0f);
    return isChanged;
#else
    return false;
#endif // _DEBUG
}
