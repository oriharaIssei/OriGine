#include "Material.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void Material::UpdateUvMatrix() {
    uvMat_ = MakeMatrix::Affine({uvTransform_.scale_, 1}, {0.f, 0.f, uvTransform_.rotate_}, {uvTransform_.translate_, 0.f});
}

void Material::Initialize(GameEntity* /*_entity*/) {
    uvTransform_ = {};
    uvMat_       = MakeMatrix::Identity();

    color_ = {1.f, 1.f, 1.f, 1.f};

    enableLighting_         = false;
    shininess_              = 0.f;
    environmentCoefficient_ = 0.1f;
    specularColor_          = {1.f, 1.f, 1.f};
}

void Material::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    ImGui::Text("Color");
    ColorEditGuiCommand("##color" + _parentLabel, color_);
    ImGui::Text("uvScale");
    DragGuiVectorCommand<2, float>("##uvScale" + _parentLabel, uvTransform_.scale_, 0.01f);
    ImGui::Text("uvRotate");
    DragGuiCommand<float>("##uvRotate" + _parentLabel, uvTransform_.rotate_, 0.01f);
    ImGui::Text("uvTranslate");
    DragGuiVectorCommand<2, float>("##uvTranslate" + _parentLabel, uvTransform_.translate_, 0.01f);
    ImGui::Text("isLightUse");
    CheckBoxCommand("##isLightUse" + _parentLabel, enableLighting_);

    ImGui::Text("shininess");
    std::string label = "##shininess" + _parentLabel;
    ImGui::DragFloat(label.c_str(), &shininess_, 0.01f);
    ImGui::Text("specularColor");
    label = "##specularColor" + _parentLabel;
    ColorEditGuiCommand(label.c_str(), specularColor_);

    ImGui::Text("EnvironmentCoefficient");
    DragGuiCommand("##EnvironmentCoefficient" + _parentLabel, environmentCoefficient_, 0.01f, 0.0f);
#endif // _DEBUG
}

void Material::Finalize() {
    uvTransform_ = {};
    uvMat_       = MakeMatrix::Identity();

    color_ = {1.f, 1.f, 1.f, 1.f};

    enableLighting_         = false;
    shininess_              = 0.f;
    environmentCoefficient_ = 0.1f;
    specularColor_          = {1.f, 1.f, 1.f};
}

void to_json(nlohmann::json& j, const Material& m) {
    to_json<2, float>(j["uvScale"], m.uvTransform_.scale_);
    to_json(j["uvRotate"], m.uvTransform_.rotate_);
    to_json<2, float>(j["uvTranslate"], m.uvTransform_.translate_);

    to_json<4, float>(j["color"], m.color_);

    j["enableLighting"] = static_cast<bool>(m.enableLighting_);
    j["shininess"]      = m.shininess_;
    to_json<3, float>(j["specularColor"], m.specularColor_);
}

void from_json(const nlohmann::json& j, Material& m) {
    j.at("uvScale").get_to(m.uvTransform_.scale_);
    j.at("uvRotate").get_to(m.uvTransform_.rotate_);
    j.at("uvTranslate").get_to(m.uvTransform_.translate_);
    j.at("color").get_to(m.color_);
    j.at("enableLighting").get_to(m.enableLighting_);
    j.at("shininess").get_to(m.shininess_);
    j.at("specularColor").get_to(m.specularColor_);
}
