#include "DirectionalLight.h"

#ifdef _DEBUG
/// externals
#include "imgui/imgui.h"
/// util
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void DirectionalLight::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGuiCommand("Color##" + _parentLabel, color);
    DragGuiCommand<float>("Intensity##" + _parentLabel, intensity, 0.01f, 0.1f);

    DragGuiVectorCommand<3, float>("Direction##" + _parentLabel, direction, 0.01f, {}, {}, "%.3f", [](Vector<3, float>* _d) { *_d = Vec3f(*_d).normalize(); });
    direction = direction.normalize();

    DragGuiCommand<float>("Angular Radius##" + _parentLabel, angularRadius, 0.0001f, {}, {}, "%.4f");

#endif // _DEBUG
}
