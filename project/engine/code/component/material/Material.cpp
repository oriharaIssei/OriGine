#include "Material.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void Material::UpdateUvMatrix() {
    uvMat_ = MakeMatrix::Affine(uvScale_, uvRotate_, uvTranslate_);
}

#ifdef _DEBUG
void Material::DebugGui() {
    ImGui::Text("Color");
    ColorEditGuiCommand("##color", color_);
    ImGui::Text("uvScale");
    DragGuiVectorCommand<3, float>("##uvScale", uvScale_, 0.01f);
    ImGui::Text("uvRotate");
    DragGuiVectorCommand<3, float>("##uvRotate", uvRotate_, 0.01f);
    ImGui::Text("uvTranslate");
    DragGuiVectorCommand<3, float>("##uvTranslate", uvTranslate_, 0.01f);
    ImGui::Text("isLightUse");
    CheckBoxCommand("##isLightUse", enableLighting_);

    ImGui::Text("shininess");
    ImGui::DragFloat("##shininess", &shininess_, 0.01f);
    ImGui::Text("specularColor");
    ColorEditGuiCommand("##specularColor", specularColor_);
}
#endif // _DEBUG

void to_json(nlohmann::json& j, const Material& m) {
    to_json<3, float>(j["uvScale"], m.uvScale_);
    to_json<3, float>(j["uvRotate"], m.uvRotate_);
    to_json<3, float>(j["uvTranslate"], m.uvTranslate_);

    to_json<4, float>(j["color"], m.color_);

    j["enableLighting"] = static_cast<bool>(m.enableLighting_);
    j["shininess"]      = m.shininess_;
    to_json<3, float>(j["specularColor"], m.specularColor_);
}

void from_json(const nlohmann::json& j, Material& m) {
    j.at("uvScale").get_to(m.uvScale_);
    j.at("uvRotate").get_to(m.uvRotate_);
    j.at("uvTranslate").get_to(m.uvTranslate_);
    j.at("color").get_to(m.color_);
    j.at("enableLighting").get_to(m.enableLighting_);
    j.at("shininess").get_to(m.shininess_);
    j.at("specularColor").get_to(m.specularColor_);
}
