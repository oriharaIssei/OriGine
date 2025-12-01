#include "GradationComponent.h"

#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"

#ifdef _DEBUG
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void to_json(nlohmann::json& j, const GradationComponent& _g) {
    j["isActive"] = _g.isActive_;

    j["materialIndex"] = _g.materialIndex_;

    j["centerUv"]  = _g.paramBuff_.openData_.centerUv;
    j["direction"] = _g.paramBuff_.openData_.direction;
    j["scale"]     = _g.paramBuff_.openData_.scale;
    j["pow"]       = _g.paramBuff_.openData_.pow;

    j["colorChannel"]  = static_cast<int32_t>(_g.paramBuff_.openData_.colorChannel);
    j["gradationType"] = static_cast<int32_t>(_g.paramBuff_.openData_.gradationType);
}

void from_json(const nlohmann::json& j, GradationComponent& _g) {
    if (j.contains("centerUv")) {
        j.at("centerUv").get_to(_g.paramBuff_.openData_.centerUv);
    }
    if (j.contains("direction")) {
        j.at("direction").get_to(_g.paramBuff_.openData_.direction);
    }
    if (j.contains("scale")) {
        j.at("scale").get_to(_g.paramBuff_.openData_.scale);
    }
    if (j.contains("pow")) {
        j.at("pow").get_to(_g.paramBuff_.openData_.pow);
    }

    _g.materialIndex_ = j.value("materialIndex", -1);
    _g.paramBuff_.openData_.colorChannel  = static_cast<ColorChannel>(j.value("colorChannel", 0));
    _g.paramBuff_.openData_.gradationType = static_cast<GradationType>(j.value("gradationType", 0));
}

void GradationComponent::Initialize(Entity* /*_entity*/) {
    auto& device = Engine::GetInstance()->GetDxDevice()->device_;
    paramBuff_.CreateBuffer(device);
    materialBuff_.CreateBuffer(device);
}

void GradationComponent::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    auto* materials = _scene->GetComponents<Material>(_entity);
    if (materials) {
        int32_t maxMaterialIndex = static_cast<int32_t>(materials->size());
        InputGuiCommand<int32_t>("Material Index##" + _parentLabel, materialIndex_, "%d", [this, maxMaterialIndex](int32_t* /*_newVal*/) {
            materialIndex_ = std::clamp(materialIndex_, 0, maxMaterialIndex);
        });
    } else {
        if (materialIndex_ > 0) {
            auto command = std::make_unique<SetterCommand<int32_t>>(&materialIndex_, -1);
            EditorController::GetInstance()->PushCommand(std::move(command));
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
