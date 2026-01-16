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

void RadialBlurParam::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
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

void OriGine::to_json(nlohmann::json& _j, const RadialBlurParam& _comp) {
    _j = nlohmann::json{
        {"isActive", _comp.isActive_},
        {"center", _comp.constantBuffer_.openData_.center_},
        {"width", _comp.constantBuffer_.openData_.width_}};
}

void OriGine::from_json(const nlohmann::json& _j, RadialBlurParam& _comp) {
    _j.at("isActive").get_to(_comp.isActive_);
    _j.at("center").get_to(_comp.constantBuffer_.openData_.center_);
    _j.at("width").get_to(_comp.constantBuffer_.openData_.width_);
}
