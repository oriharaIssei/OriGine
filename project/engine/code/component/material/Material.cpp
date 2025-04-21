#include "Material.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

const char* isLightUse[] = {
    "False",
    "True",
};

void Material::UpdateUvMatrix() {
    uvMat_ = MakeMatrix::Affine(uvScale_, uvRotate_, uvTranslate_);
}

#ifdef _DEBUG
void Material::DebugGui() {
    ImGui::Text("Color");
    ImGui::ColorEdit4("color", color_.v);
    ImGui::Text("uvScale");
    ImGui::DragFloat3("uvScale", uvScale_.v, 0.01f);
    ImGui::Text("uvRotate");
    ImGui::DragFloat3("uvRotate", uvRotate_.v, 0.01f);
    ImGui::Text("uvTranslate");
    ImGui::DragFloat3("uvTranslate", uvTranslate_.v, 0.01f);
    ImGui::Text("isLightUse");
    if (ImGui::Combo("isLightUse", &enableLighting_, isLightUse, IM_ARRAYSIZE(isLightUse))) {
        UpdateUvMatrix();
    }
    ImGui::Text("shininess");
    ImGui::DragFloat("shininess", &shininess_, 0.01f);
    ImGui::Text("specularColor");
    ImGui::ColorEdit3("specularColor", specularColor_.v);
}
#endif // _DEBUG
