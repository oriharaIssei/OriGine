#include "GrayscaleComponent.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// グレースケール度合いを渡すための定数バッファを GPU 上に確保する
/// </summary>
void OriGine::GrayscaleComponent::Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {
    constantBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}
void OriGine::GrayscaleComponent::Finalize() {}

void OriGine::GrayscaleComponent::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    CheckBoxCommand("Is Enabled##" + _parentLabel, isEnabled_);
    DragGuiCommand("Amount##" + _parentLabel, constantBuffer_.openData_.amount, 0.01f, 0.0f, 1.0f);
#endif // _DEBUG
}

/// <summary>
/// GrayscaleComponent を JSON へ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const GrayscaleComponent& _component) {
    _j = nlohmann::json{
        {"isEnabled", _component.isEnabled_},
        {"amount", _component.constantBuffer_.openData_.amount},
    };
}

/// <summary>
/// JSON から GrayscaleComponent を復元する
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, GrayscaleComponent& _component) {
    _component.isEnabled_                       = _j.at("isEnabled").get<bool>();
    _component.constantBuffer_.openData_.amount = _j.at("amount").get<float>();
}
