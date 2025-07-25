#include "RandomEffectParam.h"

/// engine
#include "engine/code/Engine.h"
// DirectX12
#include "directX12/DxDevice.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void RandomEffectParam::Initialize(GameEntity* /*_hostEntity*/) {
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

bool RandomEffectParam::Edit() {
    bool isEdited = false;

#ifdef _DEBUG
    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##BlendMode", blendModeStr[(int32_t)blendMode_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)blendMode_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&blendMode_, static_cast<BlendMode>(blendIndex)));
                isEdited = true;
                break;
            }

            blendIndex++;
        }
        ImGui::EndCombo();
    }
    isEdited |= DragGuiCommand("MaxTime", maxTime_, 0.01f, 0.01f);
    isEdited |= DragGuiCommand("Time", effectParamData_->time, 0.01f, maxTime_);
#endif // _DEBUG

    return isEdited;
}

void RandomEffectParam::Finalize() {
    effectParamData_.Finalize();
}

void to_json(nlohmann::json& j, const RandomEffectParam& param) {
    j = nlohmann::json{
        {"maxTime", param.maxTime_},
        {"blendMode", static_cast<int>(param.blendMode_)}};
}

void from_json(const nlohmann::json& j, RandomEffectParam& param) {
    j.at("maxTime").get_to(param.maxTime_);
    int blendModeInt;
    j.at("blendMode").get_to(blendModeInt);
    param.blendMode_ = static_cast<BlendMode>(blendModeInt);
}
