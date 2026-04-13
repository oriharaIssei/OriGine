#include "HierarchyArea.h"

#ifdef _DEBUG

/// engine
#include "editor/EditorConfig.h"
#include "engine/code/EngineConfig.h"
#include "input/InputManager.h"
#include "scene/SceneFactory.h"
#include "scene/SceneJsonRegistry.h"
#include <scene/Scene.h>

/// util
#include "EntityInspector.h"
#include "EntityInspectorCommands.h"
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"

using namespace OriGine;

HierarchyArea::HierarchyArea(SceneEditorWindow* _window) : Editor::Area(nameof<HierarchyArea>()), parentWindow_(_window) {}

HierarchyArea::~HierarchyArea() {}

void HierarchyArea::Initialize() {
    AddRegion(std::make_shared<EntityHierarchyRegion>(this));
}

void HierarchyArea::Finalize() {
    Editor::Area::Finalize();
}

EntityHierarchyRegion::EntityHierarchyRegion(HierarchyArea* _parent) : Editor::Region(nameof<EntityHierarchyRegion>()), parentArea_(_parent) {}
EntityHierarchyRegion::~EntityHierarchyRegion() {}

void EntityHierarchyRegion::Initialize() {}

void EntityHierarchyRegion::DrawGui() {
    ImGui::SeparatorText("Entity Hierarchy");
    Scene* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        ImGui::SeparatorText("No current scene found.");
        return;
    }
    auto& entityRepository = currentScene->GetEntityRepositoryRef()->GetEntities();
    if (entityRepository.empty()) {
        ImGui::SeparatorText("No entities in the current scene.");
        return;
    }

    // Entity の作成,削除
    if (ImGui::Button("+ Entity")) {
        // 新しいエンティティを作成
        //! TODO : 初期Entity名を設定できるように
        auto command = std::make_unique<CreateEntityCommand>(parentArea_, "Entity");
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }
    if (ImGui::Button("+ EntityFromFile")) {
        std::string directory, filename;
        if (!myfs::SelectFileDialog(kApplicationResourceDirectory + "/entities", directory, filename, {"ent"}, true)) {
            return; // キャンセルされた場合は何もしない
        }
        // ファイルからエンティティを読み込む
        auto command = std::make_unique<LoadEntityCommand>(parentArea_, kApplicationResourceDirectory + "/entities" + directory, filename);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    const auto& keyboardInput = InputManager::GetInstance()->GetKeyboard();

    /// コピー & ペースト
    if (parentArea_->IsFocused().Current()) {
        if (keyboardInput->IsPress(Key::L_CTRL) || keyboardInput->IsPress(Key::R_CTRL)) {
            // コピー
            if (keyboardInput->IsTrigger(Key::C)) {
                auto command = std::make_unique<CopyEntityCommand>(this);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
            // ペースト
            if (keyboardInput->IsTrigger(Key::V)) {
                auto command = std::make_unique<PasteEntityCommand>(this);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
        }
    }

    ImGui::InputText("Search", &searchBuff_, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SeparatorText("Entities");

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    const auto& c               = EditorConfig::Color::kWinSelectColor;
    ImVec4 winSelectColor       = ImVec4(c[R], c[G], c[B], c[A]); // Windows風の青
    ImVec4 winSelectHeaderColor = ImVec4(c[R], c[G], c[B], EditorConfig::Color::kWinSelectHeaderAlpha);
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, winSelectHeaderColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    // 選択状態のエンティティIDを取得
    if (searchBuff_.empty()) {
        for (const auto& entity : entityRepository) {
            if (!entity.IsAlive()) {
                continue; // 無効なエンティティはスキップ
            }

            EntityHandle entityHandle = entity.GetHandle();
            std::string uniqueId      = entity.GetUniqueID();

            // 選択状態か判定
            bool isSelected = std::find(selectedEntityHandles_.begin(), selectedEntityHandles_.end(), entityHandle) != selectedEntityHandles_.end();

            // Selectableで表示
            if (ImGui::Selectable(uniqueId.c_str(), isSelected)) {
                // Shiftキーで複数選択、そうでなければ単一選択
                if (ImGui::GetIO().KeyShift) {
                    if (!isSelected) {
                        // まだ選択されていなければ追加
                        auto command = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    } else {
                        // すでに選択されていれば解除
                        auto command = std::make_unique<RemoveSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    }
                } else {
                    // Shiftキーが押されていない場合は選択をクリアしてから追加
                    auto clearCommand = std::make_unique<ClearSelectedEntitiesCommand>(this);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(clearCommand));
                    auto addCommand = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(addCommand));

                    auto& parentWindowHasAreas  = parentArea_->GetParentWindow()->GetAreas();
                    auto entityInspectorAreaItr = parentWindowHasAreas.find("EntityInspectorArea");
                    if (entityInspectorAreaItr == parentWindowHasAreas.end()) {
                        LOG_ERROR("EntityInspectorArea not found in parent window.");
                        ImGui::PopStyleColor(3);
                        return;
                    }
                    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(entityInspectorAreaItr->second.get());
                    if (!entityInspectorArea) {
                        LOG_ERROR("EntityInspectorArea not found in parent window.");
                        ImGui::PopStyleColor(3);
                        return;
                    }
                    auto changedEditEntity = std::make_unique<ChangeEditEntityCommand>(
                        entityInspectorArea, entityHandle, entityInspectorArea->GetEditEntityHandle());
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(changedEditEntity));
                }
            }
        }
    } else {
        for (const auto& entity : entityRepository) {
            if (!entity.IsAlive()) {
                continue; // 無効なエンティティはスキップ
            }
            if (entity.GetUniqueID().find(searchBuff_) == std::string::npos) {
                continue; // 検索文字列にマッチしないエンティティはスキップ
            }

            EntityHandle entityHandle = entity.GetHandle();
            std::string uniqueId      = entity.GetUniqueID();

            // 選択状態か判定
            bool isSelected = std::find(selectedEntityHandles_.begin(), selectedEntityHandles_.end(), entityHandle) != selectedEntityHandles_.end();

            // Selectableで表示
            if (ImGui::Selectable(uniqueId.c_str(), isSelected)) {
                // Shiftキーで複数選択、そうでなければ単一選択
                if (ImGui::GetIO().KeyShift) {
                    if (!isSelected) {
                        // まだ選択されていなければ追加
                        auto command = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    } else {
                        // すでに選択されていれば解除
                        auto command = std::make_unique<RemoveSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    }
                } else {
                    // Shiftキーが押されていない場合は選択をクリアしてから追加
                    auto clearCommand = std::make_unique<ClearSelectedEntitiesCommand>(this);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(clearCommand));
                    auto addCommand = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(addCommand));

                    auto& parentWindowHasAreas  = parentArea_->GetParentWindow()->GetAreas();
                    auto entityInspectorAreaItr = parentWindowHasAreas.find("EntityInspectorArea");
                    if (entityInspectorAreaItr == parentWindowHasAreas.end()) {
                        LOG_ERROR("EntityInspectorArea not found in parent window.");
                        ImGui::PopStyleColor(3);
                        return;
                    }
                    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(entityInspectorAreaItr->second.get());
                    if (!entityInspectorArea) {
                        LOG_ERROR("EntityInspectorArea not found in parent window.");
                        ImGui::PopStyleColor(3);
                        return;
                    }
                    auto changedEditEntity = std::make_unique<ChangeEditEntityCommand>(
                        entityInspectorArea, entityHandle, entityInspectorArea->GetEditEntityHandle());
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(changedEditEntity));
                }
            }
        }
    }

    ImGui::PopStyleColor(3);
}

void EntityHierarchyRegion::Finalize() {}

EntityHierarchyRegion::AddSelectedEntitiesCommand::AddSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy, EntityHandle _addedEntityHandle)
    : hierarchy_(_hierarchy), addedEntityHandle_(_addedEntityHandle) {}

void EntityHierarchyRegion::AddSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityHandle_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(addedEntityHandle_);
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Execute: Entity Handle : {} is already selected.", uuids::to_string(addedEntityHandle_.uuid));
    }
}

void EntityHierarchyRegion::AddSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityHandle_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Removed entity Handle : {} from selection.", uuids::to_string(addedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Entity Handle : {} was not in selection.", uuids::to_string(addedEntityHandle_.uuid));
    }
}

EntityHierarchyRegion::RemoveSelectedEntitiesCommand::RemoveSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy, EntityHandle _removedEntityHandle)
    : hierarchy_(_hierarchy), removedEntityHandle_(_removedEntityHandle) {}

void EntityHierarchyRegion::RemoveSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityHandle_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Removed entity Handle : {} from selection.", uuids::to_string(removedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Entity Handle : {} was not in selection.", uuids::to_string(removedEntityHandle_.uuid));
    }
}

void EntityHierarchyRegion::RemoveSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityHandle_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(removedEntityHandle_);
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Added entity Handle : {} back to selection.", uuids::to_string(removedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Entity Handle : {} is already in selection.", uuids::to_string(removedEntityHandle_.uuid));
    }
}

EntityHierarchyRegion::ClearSelectedEntitiesCommand::ClearSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy)
    : hierarchy_(_hierarchy) {}

void EntityHierarchyRegion::ClearSelectedEntitiesCommand::Execute() {
    previousselectedEntityHandles_ = hierarchy_->selectedEntityHandles_; // 現在の選択状態を保存
    if (!previousselectedEntityHandles_.empty()) {
        hierarchy_->selectedEntityHandles_.clear();
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: Cleared all selected entities.");
    } else {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: No entities were selected to Clear.");
    }
}

void EntityHierarchyRegion::ClearSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (previousselectedEntityHandles_.empty()) {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Undo: No previous selection to restore.");
        return;
    }
    selectedEntityIds = previousselectedEntityHandles_; // 保存した選択状態を復元
}

EntityHierarchyRegion::CreateEntityCommand::CreateEntityCommand(HierarchyArea* _parentArea, const std::string& _entityName) {
    parentArea_ = _parentArea;
    entityName_ = _entityName;
}
void EntityHierarchyRegion::CreateEntityCommand::Execute() {
    Scene* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Execute: No current scene found.");
        return;
    }

    entityHandle_ = currentScene->GetEntityRepositoryRef()->CreateEntity(entityName_);

    LOG_DEBUG("CreateEntityCommand::Execute: Created entity with Handle '{}'.", uuids::to_string(entityHandle_.uuid));
}
void EntityHierarchyRegion::CreateEntityCommand::Undo() {
    Scene* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    currentScene->AddDeleteEntity(entityHandle_);

    LOG_DEBUG("Removed entity with UUHandle : {}.", uuids::to_string(entityHandle_.uuid));
}

EntityHierarchyRegion::LoadEntityCommand::LoadEntityCommand(HierarchyArea* _parentArea, const std::string& _directory, const std::string& _entityName) {
    parentArea_ = _parentArea;
    directory_  = _directory;
    entityName_ = _entityName;

    // 読み込み ＆ テンプレート登録
    SceneJsonRegistry::GetInstance()->LoadEntityTemplate(directory_, entityName_);
}
void EntityHierarchyRegion::LoadEntityCommand::Execute() {
    Scene* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    SceneFactory factory;
    Entity* createdEntity = factory.BuildEntityFromTemplate(currentScene, entityName_);
    entityHandle_         = createdEntity->GetHandle();

    LOG_DEBUG("Created entity with Handle : {}.", uuids::to_string(entityHandle_.uuid));
}
void EntityHierarchyRegion::LoadEntityCommand::Undo() {
    Scene* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    currentScene->AddDeleteEntity(entityHandle_);

    LOG_DEBUG("Removed entity with Handle : {}.", uuids::to_string(entityHandle_.uuid));
}
EntityHierarchyRegion::CopyEntityCommand::CopyEntityCommand(EntityHierarchyRegion* _hierarchy) : hierarchy_(_hierarchy) {}

void EntityHierarchyRegion::CopyEntityCommand::Execute() {
    if (hierarchy_->selectedEntityHandles_.empty()) {
        return;
    }
    // 既存のコピー内容をクリア
    if (!hierarchy_->copyBuffer_.empty()) {
        hierarchy_->copyBuffer_.clear();
    }

    // Dataをコピー
    Scene* currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();

    SceneFactory factory;
    for (auto entityId : hierarchy_->selectedEntityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found for copying.", uuids::to_string(entityId.uuid));
            continue;
        }
        hierarchy_->copyBuffer_.emplace_back(factory.CreateEntityJsonFromEntity(currentScene, entity));
    }
}

void EntityHierarchyRegion::CopyEntityCommand::Undo() {
    // コピーしたデータをクリア
    hierarchy_->copyBuffer_.clear();
}

EntityHierarchyRegion::PasteEntityCommand::PasteEntityCommand(EntityHierarchyRegion* _hierarchy) : hierarchy_(_hierarchy) {}

void EntityHierarchyRegion::PasteEntityCommand::Execute() {
    if (hierarchy_->copyBuffer_.empty()) {
        return;
    }
    // copyBufferのデータをシーンに貼り付け
    Scene* currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();

    // 貼り付けて生成したエンティティIDを保存(削除に利用)
    SceneFactory sceneFactory;
    for (const auto& entityJson : hierarchy_->copyBuffer_) {
        Entity* createdEntity = sceneFactory.BuildEntity(currentScene, entityJson, HandleAssignMode::GenerateNew);
        pastedEntityHandles_.emplace_back(createdEntity->GetHandle());
    }
}

void EntityHierarchyRegion::PasteEntityCommand::Undo() {
    // 貼り付けたエンティティを削除
    Scene* currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();
    for (auto entityId : pastedEntityHandles_) {
        currentScene->AddDeleteEntity(entityId);
    }
    pastedEntityHandles_.clear();
}

#endif // DEBUG
