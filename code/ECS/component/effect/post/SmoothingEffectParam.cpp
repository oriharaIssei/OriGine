#include "SmoothingEffectParam.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

/// editor
#ifdef ORIGINE_EDITOR_ENABLED
#include "myGui/MyGui.h"
#endif // DEBUG

using namespace OriGine;

void SmoothingEffectParam::Initialize(Scene* /*_scene,*/, const EntityHandle& /*_owner*/) {
    boxFilterSize_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SmoothingEffectParam::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED
    CheckBoxCommand("isActive##" + _parentLabel, isActive_);

    DragGuiVectorCommand("BoxFilter Size##" + _parentLabel, boxFilterSize_->size, 0.01f, 0.0f);

#endif // DEBUG
}

void SmoothingEffectParam::Finalize() {}

// friend 宣言は OriGine::to_json / OriGine::from_json を指すため、ここで定義する
// 実体も同じ名前空間に置かないと ADL で見つからず、シリアライズ経路に一度も乗らない
// (2026-09-22 修正。以前はここが `using namespace OriGine;` の下でグローバル名前空間に
// 定義されており、friend 宣言と別の関数として扱われていた)。
namespace OriGine {

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

} // namespace OriGine
