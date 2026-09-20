#include "PointLight.h"

#ifdef ORIGINE_EDITOR_ENABLED
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void PointLight::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiCommand<float>("Radius##" + _parentLabel, radius, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);
    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.01f, 0.0f);

    ImGui::Spacing();

    InputGuiCommand("Target Transform Index##" + _parentLabel, targetTransformIndex);

#endif // _DEBUG
}
