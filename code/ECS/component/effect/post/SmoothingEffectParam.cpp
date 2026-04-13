#include "SmoothingEffectParam.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

/// editor
#ifdef DEBUG
#include "myGui/MyGui.h"
#endif // DEBUG

using namespace OriGine;

void SmoothingEffectParam::Initialize(Scene* /*_scene,*/, EntityHandle /*_owner*/) {
    boxFilterSize_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SmoothingEffectParam::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef DEBUG
    CheckBoxCommand("isActive##" + _parentLabel, isActive_);

    DragGuiVectorCommand("BoxFilter Size##" + _parentLabel, boxFilterSize_->size, 0.01f, 0.0f);

#endif // DEBUG
}

void SmoothingEffectParam::Finalize() {}

void to_json(nlohmann::json& _j, const SmoothingEffectParam& _comp) {
    _j = nlohmann::json{
        {"isActive", _comp.isActive_},
        {"boxFilterSize", _comp.boxFilterSize_.openData_.size},
    };
}

void from_json(const nlohmann::json& _j, SmoothingEffectParam& _comp) {
    _j.at("isActive").get_to(_comp.isActive_);
    _j.at("boxFilterSize").get_to(_comp.boxFilterSize_.openData_.size);
}
