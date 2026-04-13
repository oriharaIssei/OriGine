#include "RandomEffectParam.h"

/// engine
#include "engine/code/Engine.h"
// DirectX12
#include "directX12/DxDevice.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void RandomEffectParam::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
    effectParamData_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void RandomEffectParam::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    CheckBoxCommand("Active##" + _parentLabel, isActive_);

    ImGui::Spacing();

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), kBlendModeStr[(int32_t)blendMode_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : kBlendModeStr) {
            isSelected = blendModeName == kBlendModeStr[(int32_t)blendMode_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&blendMode_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            ++blendIndex;
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    DragGuiCommand("MaxTime##" + _parentLabel, maxTime_, 0.01f, 0.01f);
    DragGuiCommand("Time##" + _parentLabel, effectParamData_->time, 0.01f, maxTime_);
#endif // _DEBUG
}

void RandomEffectParam::Finalize() {
    effectParamData_.Finalize();
}

void OriGine::to_json(nlohmann::json& _j, const RandomEffectParam& _comp) {
    _j = nlohmann::json{
        {"isActive", _comp.isActive_},
        {"maxTime", _comp.maxTime_},
        {"blendMode", static_cast<int>(_comp.blendMode_)}};
}

void OriGine::from_json(const nlohmann::json& _j, RandomEffectParam& _comp) {
    _j.at("isActive").get_to(_comp.isActive_);

    _j.at("maxTime").get_to(_comp.maxTime_);
    int blendModeInt;
    _j.at("blendMode").get_to(blendModeInt);
    _comp.blendMode_ = static_cast<BlendMode>(blendModeInt);
}
