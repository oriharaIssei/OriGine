#include "MaterialEffectPipeLine.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "asset/AssetSystem.h"
#include "asset/TextureAsset.h"
#include "EngineInclude.h"
#include "scene/Scene.h"

/// ECS
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/GradationComponent.h"
#include "component/material/Material.h"

/// util
#include "myFileSystem/MyFileSystem.h"

/// editor
#ifdef _DEBUG
#include "editor/IEditor.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

MaterialEffectPipeLine::MaterialEffectPipeLine() {}
MaterialEffectPipeLine::~MaterialEffectPipeLine() {}

void MaterialEffectPipeLine::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    if (!baseTexturePath_.empty()) {
        LoadBaseTexture(baseTexturePath_);
    }
}

void MaterialEffectPipeLine::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    auto askLoadTexture = [this]([[maybe_unused]] const std::string& _parentLabel) {
        bool ask          = false;
        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());
        ask |= ImGui::ImageButton(
            ImTextureID(AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(baseTextureId_).srv.GetGpuHandle().ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };
    ImGui::Text("Texture Directory: %s", baseTexturePath_.c_str());
    if (askLoadTexture(_parentLabel)) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto setPath = std::make_unique<SetterCommand<std::string>>(&baseTexturePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
            CommandCombo commandCombo;
            commandCombo.AddCommand(std::move(setPath));
            commandCombo.SetFuncOnAfterCommand([this]() {
                baseTextureId_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(baseTexturePath_);
            },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    };

    std::string label          = "MaterialIndex##" + _parentLabel;
    auto& materials            = _scene->GetComponents<Material>(_entity);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size());

    if (entityMaterialSize <= 0) {
        ImGui::InputInt(label.c_str(), &materialIndex_, 0, 0, ImGuiInputTextFlags_ReadOnly);
    } else {
        InputGuiCommand(label, materialIndex_);
        materialIndex_ = std::clamp(materialIndex_, 0, entityMaterialSize - 1);
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        if (materialIndex_ >= 0 && materialIndex_ < materials.size()) {
            materials[materialIndex_].Edit(_scene, _entity, label);
        } else {
            ImGui::Text("Material is null.");
        }
        ImGui::TreePop();
    }

    InputGuiCommand("isActive##" + _parentLabel, isActive_);
    InputGuiCommand("priority##" + _parentLabel, priority_);

    label = "AddEffectEntity##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        auto command = std::make_unique<AddElementCommand<std::vector<EffectEntityData>>>(&effectEntityData_, MaterialEffectPipeLine::EffectEntityData());
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    label = "ClearEffectEntity##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        auto command = std::make_unique<ClearCommand<std::vector<EffectEntityData>>>(&effectEntityData_);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    /// Effectを持っているEntity一覧
    std::vector<std::vector<EntityHandle>> effectEntityIds;

    effectEntityIds.emplace_back(std::vector<EntityHandle>());
    effectEntityIds[0].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<DissolveEffectParam>()->GetSlots()) {
        effectEntityIds[0].emplace_back(indexBind.owner);
    }
    effectEntityIds.emplace_back(std::vector<EntityHandle>());
    effectEntityIds[1].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<DistortionEffectParam>()->GetSlots()) {
        effectEntityIds[1].emplace_back(indexBind.owner);
    }
    effectEntityIds.emplace_back(std::vector<EntityHandle>());
    effectEntityIds[2].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<GradationComponent>()->GetSlots()) {
        effectEntityIds[2].emplace_back(indexBind.owner);
    }

    ImGui::Spacing();

    for (size_t i = 0; i < effectEntityData_.size(); ++i) {
        std::string effectLabel = "EffectType##" + std::to_string(i) + _parentLabel;
        int32_t effectTypeInt   = static_cast<int32_t>(effectEntityData_[i].effectType);

        // type選択コンボボックス
        if (ImGui::BeginCombo(effectLabel.c_str(), materialEffectString[effectTypeInt].c_str())) {
            for (int j = 0; j < static_cast<int>(MaterialEffectType::Count); ++j) {
                bool isSelected = (effectTypeInt == j);
                if (ImGui::Selectable(materialEffectString[j].c_str(), isSelected)) {
                    auto command = std::make_unique<SetterCommand<MaterialEffectType>>(
                        &effectEntityData_[i].effectType,
                        static_cast<MaterialEffectType>(j));
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        std::string removeButtonLabel = "X##" + std::to_string(i) + _parentLabel;
        if (ImGui::Button(removeButtonLabel.c_str())) {
            auto command = std::make_unique<EraseElementCommand<std::vector<EffectEntityData>>>(&effectEntityData_, effectEntityData_.begin() + i);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
        }
        ImGui::SameLine();

        // entity選択コンボボックス
        std::string entityIdLabel = "EntityID##" + std::to_string(i) + _parentLabel;
        Entity* entity            = _scene->GetEntity(effectEntityData_[i].entityHandle);

        std::string entityName = "NULL";
        if (entity) {
            entityName = entity->GetUniqueID();
        }
        // entity選択コンボボックス
        if (ImGui::BeginCombo(entityIdLabel.c_str(), entityName.c_str())) {
            for (int j = 0; j < effectEntityIds[effectTypeInt].size(); ++j) {
                bool isSelected      = (effectEntityData_[i].entityHandle == effectEntityIds[effectTypeInt][j]);
                Entity* effectEntity = _scene->GetEntity(effectEntityIds[effectTypeInt][j]);
                if (!effectEntity) {
                    continue;
                }
                if (ImGui::Selectable(effectEntity->GetUniqueID().c_str(), isSelected)) {
                    auto command = std::make_unique<SetterCommand<EntityHandle>>(
                        &effectEntityData_[i].entityHandle,
                        effectEntityIds[effectTypeInt][j]);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
#endif // _DEBUG
}

void MaterialEffectPipeLine::Finalize() {}

void MaterialEffectPipeLine::LoadBaseTexture(const std::string& _path) {
    baseTexturePath_ = _path;
    baseTextureId_   = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(baseTexturePath_);
}

void OriGine::to_json(nlohmann::json& _j, const MaterialEffectPipeLine& _comp) {
    _j["baseTexturePath"] = _comp.baseTexturePath_;
    _j["isActive"]        = _comp.isActive_;
    _j["priority"]        = _comp.priority_;
    _j["materialIndex"]   = _comp.materialIndex_;

    nlohmann::json effectList = nlohmann::json::array();
    for (const auto& effectData : _comp.effectEntityData_) {
        nlohmann::json effectJson;
        effectJson["effectType"] = static_cast<int>(effectData.effectType);
        effectJson["handle"]     = effectData.entityHandle;
        effectList.push_back(effectJson);
    }
    _j["effectEntityIdList"] = effectList;
}

void OriGine::from_json(const nlohmann::json& _j, MaterialEffectPipeLine& _comp) {
    _j.at("baseTexturePath").get_to(_comp.baseTexturePath_);

    _j.at("isActive").get_to(_comp.isActive_);
    if (_j.contains("priority")) {
        _j.at("priority").get_to(_comp.priority_);
    }

    _j.at("materialIndex").get_to(_comp.materialIndex_);

    _comp.effectEntityData_.clear();
    if (_j.contains("effectEntityIdList")) {
        for (const auto& effectJson : _j.at("effectEntityIdList")) {
            MaterialEffectPipeLine::EffectEntityData entData;
            if (effectJson.contains("effectType")) {
                int effectTypeInt = 0;
                effectJson.at("effectType").get_to(effectTypeInt);
                entData.effectType = static_cast<MaterialEffectType>(effectTypeInt);
            }
            if (effectJson.contains("handle")) {
                effectJson.at("handle").get_to(entData.entityHandle);
            }
            _comp.effectEntityData_.push_back(entData);
        }
    }
}
