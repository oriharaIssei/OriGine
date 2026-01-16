#include "GradationComponent.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"
// directX12
#include "directX12/DxDevice.h"

/// util
#ifdef _DEBUG
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void OriGine::to_json(nlohmann::json& _j, const GradationComponent& _comp) {
    _j["isActive"] = _comp.isActive_;

    _j["materialIndex"] = _comp.materialIndex_;

    _j["centerUv"]  = _comp.paramBuff_.openData_.centerUv;
    _j["direction"] = _comp.paramBuff_.openData_.direction;
    _j["scale"]     = _comp.paramBuff_.openData_.scale;
    _j["pow"]       = _comp.paramBuff_.openData_.pow;

    _j["colorChannel"]  = static_cast<int32_t>(_comp.paramBuff_.openData_.colorChannel);
    _j["gradationType"] = static_cast<int32_t>(_comp.paramBuff_.openData_.gradationType);
}

void OriGine::from_json(const nlohmann::json& _j, GradationComponent& _comp) {
    if (_j.contains("centerUv")) {
        _j.at("centerUv").get_to(_comp.paramBuff_.openData_.centerUv);
    }
    if (_j.contains("direction")) {
        _j.at("direction").get_to(_comp.paramBuff_.openData_.direction);
    }
    if (_j.contains("scale")) {
        _j.at("scale").get_to(_comp.paramBuff_.openData_.scale);
    }
    if (_j.contains("pow")) {
        _j.at("pow").get_to(_comp.paramBuff_.openData_.pow);
    }

    _comp.materialIndex_                     = _j.value("materialIndex", -1);
    _comp.paramBuff_.openData_.colorChannel  = static_cast<ColorChannel>(_j.value("colorChannel", 0));
    _comp.paramBuff_.openData_.gradationType = static_cast<GradationType>(_j.value("gradationType", 0));
}

void GradationComponent::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    auto& device = Engine::GetInstance()->GetDxDevice()->device_;
    paramBuff_.CreateBuffer(device);
    materialBuff_.CreateBuffer(device);
}

void GradationComponent::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _owner, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    auto& materials = _scene->GetComponents<Material>(_owner);
    if (!materials.empty()) {
        int32_t maxMaterialIndex = static_cast<int32_t>(materials.size()) - 1;
        InputGuiCommand<int32_t>("Material Index##" + _parentLabel, materialIndex_, "%d", [this, maxMaterialIndex](int32_t* /*_newVal*/) {
            materialIndex_ = std::clamp(materialIndex_, 0, maxMaterialIndex);
        });
    } else {
        if (materialIndex_ > 0) {
            auto command = std::make_unique<SetterCommand<int32_t>>(&materialIndex_, -1);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

    ImGui::Spacing();

    InputGuiCommand("IsActive##" + _parentLabel, isActive_);

    constexpr const char* gradationTypeItems[] = {"Directional", "Radial"};
    int32_t gradationType                      = static_cast<int32_t>(paramBuff_.openData_.gradationType);
    if (ImGui::Combo(("GradationType##" + _parentLabel).c_str(), &gradationType, gradationTypeItems, IM_ARRAYSIZE(gradationTypeItems))) {
        paramBuff_.openData_.gradationType = static_cast<GradationType>(gradationType);
    }
    constexpr const char* channelItems[] = {"R", "G", "B", "A"};
    int32_t colorChannel                 = static_cast<int32_t>(paramBuff_.openData_.colorChannel);
    if (ImGui::Combo(("ColorChannel##" + _parentLabel).c_str(), &colorChannel, channelItems, IM_ARRAYSIZE(channelItems))) {
        paramBuff_.openData_.colorChannel = static_cast<ColorChannel>(colorChannel);
    }

    ImGui::Spacing();

    DragGuiVectorCommand("Center UV##" + _parentLabel, paramBuff_.openData_.centerUv, 0.01f);
    DragGuiVectorCommand("Direction##" + _parentLabel, paramBuff_.openData_.direction, 0.01f);
    paramBuff_.openData_.direction = paramBuff_.openData_.direction.normalize();

    ImGui::Spacing();

    DragGuiCommand("Scale##" + _parentLabel, paramBuff_.openData_.scale, 0.01f);
    DragGuiCommand("Power##" + _parentLabel, paramBuff_.openData_.pow, 0.01f);
#endif // _DEBUG
}

void GradationComponent::Finalize() {
    paramBuff_.Finalize();
    materialBuff_.Finalize();
}
