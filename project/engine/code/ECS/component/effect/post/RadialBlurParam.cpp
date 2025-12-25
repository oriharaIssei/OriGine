#include "RadialBlurParam.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

RadialBlurParam::RadialBlurParam() {}
RadialBlurParam::~RadialBlurParam() {}

void RadialBlurParam:: Initialize(Scene* /*_scene,*/, EntityHandle /*_owner*/) {
    if (isActive_) {
        constantBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        constantBuffer_.ConvertToBuffer();
    }
}

void RadialBlurParam::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    if (CheckBoxCommand("Active##" + _parentLabel, isActive_)) {
        constantBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
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
    constantBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    constantBuffer_.ConvertToBuffer();
}
void RadialBlurParam::Stop() {
    isActive_ = false;
    constantBuffer_.Finalize();
}

void OriGine::to_json(nlohmann::json& j, const RadialBlurParam& param) {
    j = nlohmann::json{
        {"isActive", param.isActive_},
        {"center", param.constantBuffer_.openData_.center_},
        {"width", param.constantBuffer_.openData_.width_}};
}

void OriGine::from_json(const nlohmann::json& j, RadialBlurParam& param) {
    j.at("isActive").get_to(param.isActive_);
    j.at("center").get_to(param.constantBuffer_.openData_.center_);
    j.at("width").get_to(param.constantBuffer_.openData_.width_);
}
