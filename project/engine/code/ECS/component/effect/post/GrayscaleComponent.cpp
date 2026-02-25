#include "GrayscaleComponent.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void OriGine::GrayscaleComponent::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    constantBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}
void OriGine::GrayscaleComponent::Finalize() {}

void OriGine::GrayscaleComponent::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    CheckBoxCommand("Is Enabled##" + _parentLabel, isEnabled_);
    DragGuiCommand("Amount##" + _parentLabel, constantBuffer_.openData_.amount, 0.01f, 0.0f, 1.0f);
#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const GrayscaleComponent& _component) {
    _j = nlohmann::json{
        {"isEnabled", _component.isEnabled_},
        {"amount", _component.constantBuffer_.openData_.amount},
    };
}

void OriGine::from_json(const nlohmann::json& _j, GrayscaleComponent& _component) {
    _component.isEnabled_                       = _j.at("isEnabled").get<bool>();
    _component.constantBuffer_.openData_.amount = _j.at("amount").get<float>();
}
