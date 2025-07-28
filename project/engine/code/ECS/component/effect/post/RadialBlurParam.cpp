#include "RadialBlurParam.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

RadialBlurParam::RadialBlurParam() {}

RadialBlurParam::~RadialBlurParam() {}

void RadialBlurParam::Initialize(GameEntity* /*_entity*/) {
    if (isActive_) {
        constantBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        constantBuffer_.ConvertToBuffer();
    }
}

void RadialBlurParam::Edit(Scene* /*_scene*/, GameEntity* /*_entity*/, const std::string& _parentLabel) {

#ifdef _DEBUG
    if (CheckBoxCommand("Active##" + _parentLabel, isActive_)) {
        constantBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        constantBuffer_.ConvertToBuffer();
    }
    ImGui::Spacing();
    DragGuiVectorCommand("Center##" + _parentLabel, constantBuffer_.openData_.center_);
    DragGuiCommand("Width##" + _parentLabel, constantBuffer_.openData_.width_, 0.01f);

#endif // _DEBUG
}

void RadialBlurParam::Finalize() {
    constantBuffer_.Finalize();
}

void RadialBlurParam::Play() {
    isActive_ = true;
    constantBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    constantBuffer_.ConvertToBuffer();
}
void RadialBlurParam::Stop() {
    isActive_ = false;
    constantBuffer_.Finalize();
}

void to_json(nlohmann::json& j, const RadialBlurParam& param) {
    j = nlohmann::json{
        {"isActive", param.isActive_},
        {"center", param.constantBuffer_.openData_.center_},
        {"width", param.constantBuffer_.openData_.width_}};
}

void from_json(const nlohmann::json& j, RadialBlurParam& param) {
    j.at("isActive").get_to(param.isActive_);
    j.at("center").get_to(param.constantBuffer_.openData_.center_);
    j.at("width").get_to(param.constantBuffer_.openData_.width_);
}
