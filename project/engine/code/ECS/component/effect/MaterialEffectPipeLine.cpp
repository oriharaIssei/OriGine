#include "MaterialEffectPipeLine.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"

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

MaterialEffectPipeLine::MaterialEffectPipeLine() {}

MaterialEffectPipeLine::~MaterialEffectPipeLine() {}

void MaterialEffectPipeLine::Initialize(Entity* /*_entity*/) {
    if (!baseTexturePath_.empty()) {
        LoadBaseTexture(baseTexturePath_);
    }
}

void MaterialEffectPipeLine::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    auto askLoadTexture = [this]([[maybe_unused]] const std::string& _parentLabel) {
        bool ask          = false;
        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());
        ask |= ImGui::ImageButton(
            ImTextureID(TextureManager::GetDescriptorGpuHandle(baseTextureId_).ptr),
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
                baseTextureId_ = TextureManager::LoadTexture(baseTexturePath_);
            },
                true);
            EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    };

    std::string label          = "MaterialIndex##" + _parentLabel;
    auto materials             = _scene->GetComponents<Material>(_entity);
    int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;

    if (entityMaterialSize <= 0) {
        ImGui::InputInt(label.c_str(), &materialIndex_, 0, 0, ImGuiInputTextFlags_ReadOnly);
    } else {
        InputGuiCommand(label, materialIndex_);
        materialIndex_ = std::clamp(materialIndex_, 0, entityMaterialSize - 1);
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        if (materials != nullptr && materialIndex_ >= 0 && materialIndex_ < materials->size()) {
            (*materials)[materialIndex_].Edit(_scene, _entity, label);
        } else {
            ImGui::Text("Material is null.");
        }
        ImGui::TreePop();
    }

    InputGuiCommand("isActive##" + _parentLabel, isActive_);
    InputGuiCommand("priority##" + _parentLabel, priority_);

    label = "AddEffectEntity##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        auto command = std::make_unique<AddElementCommand<std::vector<EffectEntityData>>>(&effectEntityIdList_, MaterialEffectPipeLine::EffectEntityData());
        EditorController::GetInstance()->PushCommand(std::move(command));
    }

    label = "ClearEffectEntity##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        auto command = std::make_unique<ClearCommand<std::vector<EffectEntityData>>>(&effectEntityIdList_);
        EditorController::GetInstance()->PushCommand(std::move(command));
    }

    /// Effectを持っているEntity一覧
    std::vector<std::vector<int32_t>> effectEntityIds;

    effectEntityIds.emplace_back(std::vector<int32_t>());
    effectEntityIds[0].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<DissolveEffectParam>()->GetEntityIndexBind()) {
        effectEntityIds[0].emplace_back(indexBind.first);
    }
    effectEntityIds.emplace_back(std::vector<int32_t>());
    effectEntityIds[1].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<DistortionEffectParam>()->GetEntityIndexBind()) {
        effectEntityIds[1].emplace_back(indexBind.first);
    }
    effectEntityIds.emplace_back(std::vector<int32_t>());
    effectEntityIds[2].reserve(10);
    for (auto& indexBind : _scene->GetComponentArray<GradationComponent>()->GetEntityIndexBind()) {
        effectEntityIds[2].emplace_back(indexBind.first);
    }

    ImGui::Spacing();

    for (size_t i = 0; i < effectEntityIdList_.size(); ++i) {
        std::string effectLabel = "EffectType##" + std::to_string(i) + _parentLabel;
        int32_t effectTypeInt   = static_cast<int32_t>(effectEntityIdList_[i].effectType);
        if (ImGui::BeginCombo(effectLabel.c_str(), materialEffectString[effectTypeInt].c_str())) {
            for (int j = 0; j < static_cast<int>(MaterialEffectType::Count); ++j) {
                bool isSelected = (effectTypeInt == j);
                if (ImGui::Selectable(materialEffectString[j].c_str(), isSelected)) {
                    auto command = std::make_unique<SetterCommand<MaterialEffectType>>(
                        &effectEntityIdList_[i].effectType,
                        static_cast<MaterialEffectType>(j));
                    EditorController::GetInstance()->PushCommand(std::move(command));
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        std::string removeButtonLabel = "X##" + std::to_string(i) + _parentLabel;
        if (ImGui::Button(removeButtonLabel.c_str())) {
            auto command = std::make_unique<EraseElementCommand<std::vector<EffectEntityData>>>(&effectEntityIdList_, effectEntityIdList_.begin() + i);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
        ImGui::SameLine();

        std::string entityIdLabel = "EntityID##" + std::to_string(i) + _parentLabel;
        std::string entityName    = _scene->GetEntity(effectEntityIdList_[i].entityID) != nullptr ? std::to_string(effectEntityIdList_[i].entityID) : "NULL";
        if (ImGui::BeginCombo(entityIdLabel.c_str(), entityName.c_str())) {
            for (int j = 0; j < effectEntityIds[effectTypeInt].size(); ++j) {
                bool isSelected = (effectEntityIdList_[i].entityID == effectEntityIds[effectTypeInt][j]);
                if (ImGui::Selectable(_scene->GetEntity(effectEntityIds[effectTypeInt][j])->GetUniqueID().c_str(), isSelected)) {
                    auto command = std::make_unique<SetterCommand<int32_t>>(
                        &effectEntityIdList_[i].entityID,
                        effectEntityIds[effectTypeInt][j]);
                    EditorController::GetInstance()->PushCommand(std::move(command));
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
    baseTextureId_   = TextureManager::LoadTexture(baseTexturePath_);
}

void to_json(nlohmann::json& j, const MaterialEffectPipeLine& c) {
    j["baseTexturePath"] = c.baseTexturePath_;
    j["isActive"]        = c.isActive_;
    j["priority"]        = c.priority_;
    j["materialIndex"]   = c.materialIndex_;

    nlohmann::json effectList = nlohmann::json::array();
    for (const auto& effectData : c.effectEntityIdList_) {
        nlohmann::json effectJson;
        effectJson["effectType"] = static_cast<int>(effectData.effectType);
        effectJson["entityID"]   = effectData.entityID;
        effectList.push_back(effectJson);
    }
    j["effectEntityIdList"] = effectList;
}

void from_json(const nlohmann::json& j, MaterialEffectPipeLine& c) {
    j.at("baseTexturePath").get_to(c.baseTexturePath_);

    j.at("isActive").get_to(c.isActive_);
    if (j.contains("priority")) {
        j.at("priority").get_to(c.priority_);
    }

    j.at("materialIndex").get_to(c.materialIndex_);

    c.effectEntityIdList_.clear();
    if (j.contains("effectEntityIdList")) {
        for (const auto& effectJson : j.at("effectEntityIdList")) {
            MaterialEffectPipeLine::EffectEntityData effectData;
            if (effectJson.contains("effectType")) {
                int effectTypeInt = 0;
                effectJson.at("effectType").get_to(effectTypeInt);
                effectData.effectType = static_cast<MaterialEffectType>(effectTypeInt);
            }
            if (effectJson.contains("entityID")) {
                effectJson.at("entityID").get_to(effectData.entityID);
            }
            c.effectEntityIdList_.push_back(effectData);
        }
    }
}
