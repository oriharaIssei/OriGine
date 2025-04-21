#include "DirectionalLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

bool DirectionalLight::Edit() {
#ifdef _DEBUG
    bool isChanged = false;

    isChanged |= ImGui::Checkbox("Active", &isActive_);

    ImGui::Spacing();

    isChanged |= ImGui::ColorEdit3("Color", color_.v);
    isChanged |= ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);
    if (ImGui::DragFloat3("Direction", direction_.v, 0.01f)) {
        direction_ = direction_.normalize();
        isChanged  = true;
    }

    return isChanged;
#else
    return false;
#endif // _DEBUG
}
