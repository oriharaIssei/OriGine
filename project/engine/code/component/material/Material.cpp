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
