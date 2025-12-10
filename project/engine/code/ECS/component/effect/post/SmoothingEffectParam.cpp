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

void SmoothingEffectParam::Initialize(Entity* /*_entity*/) {
    boxFilterSize_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SmoothingEffectParam::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef DEBUG
    CheckBoxCommand("isActive##" + _parentLabel, isActive_);

    DragGuiVectorCommand("BoxFilter Size##" + _parentLabel, boxFilterSize_->size, 0.01f, 0.0f);

#endif // DEBUG
}

void SmoothingEffectParam::Finalize() {}

void to_json(nlohmann::json& j, const SmoothingEffectParam& c) {
    j = nlohmann::json{
        {"isActive", c.isActive_},
        {"boxFilterSize", c.boxFilterSize_.openData_.size},
    };
}

void from_json(const nlohmann::json& j, SmoothingEffectParam& c) {
    j.at("isActive").get_to(c.isActive_);
    j.at("boxFilterSize").get_to(c.boxFilterSize_.openData_.size);
}
