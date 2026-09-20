#include "SpotLight.h"

#ifdef ORIGINE_EDITOR_ENABLED
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void SpotLight::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiVectorCommand<3, float>("Position##" + _parentLabel, pos, 0.01f);
    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f::Normalize(*_d); });
    direction = Vec3f::Normalize(direction);

    DragGuiCommand<float>("Distance##" + _parentLabel, distance, 0.01f, 0.0f);
    DragGuiCommand<float>("Decay##" + _parentLabel, decay, 0.01f, 0.0f);

    ImGui::Spacing();

    DragGuiCommand<float>("CosAngle##" + _parentLabel, cosAngle, 0.01f, 0.0f, 1.0f);
    DragGuiCommand<float>("CosFalloffStart##" + _parentLabel, cosFalloffStart, 0.01f, 0.0f, 1.0f);
    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.01f, 0.0f);

    ImGui::Spacing();

    InputGuiCommand("Target Transform Index##" + _parentLabel, targetTransformIndex);

#endif // _DEBUG
}
