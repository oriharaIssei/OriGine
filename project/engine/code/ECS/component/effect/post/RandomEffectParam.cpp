#include "RandomEffectParam.h"

/// engine
#include "engine/code/Engine.h"
// DirectX12
#include "directX12/DxDevice.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void RandomEffectParam::Initialize(Entity* /*_hostEntity*/) {
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->device_);
}

void RandomEffectParam::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    CheckBoxCommand("Active##" + _parentLabel, isActive_);

    ImGui::Spacing();

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)blendMode_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)blendMode_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
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

inline void to_json(nlohmann::json& j, const RandomEffectParam& param) {
    j = nlohmann::json{
        {"isActive", param.isActive_},
        {"maxTime", param.maxTime_},
        {"blendMode", static_cast<int>(param.blendMode_)}};
}

inline void from_json(const nlohmann::json& j, RandomEffectParam& param) {
    j.at("isActive").get_to(param.isActive_);

    j.at("maxTime").get_to(param.maxTime_);
    int blendModeInt;
    j.at("blendMode").get_to(blendModeInt);
    param.blendMode_ = static_cast<BlendMode>(blendModeInt);
}
