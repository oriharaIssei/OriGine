#include "EntityInspectorCommands.h"

#ifdef _DEBUG

/// engine
#define RESOURCE_DIRECTORY
#include "editor/EditorConfig.h"
#include "engine/code/EngineConfig.h"
#include "EngineInclude.h"

/// scene
#include "scene/SceneFactory.h"
#include "scene/SceneManager.h"

/// ECS
#include "system/SystemRunner.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/EntityInspector.h"
#include "editor/sceneEditor/SceneEditor.h"

using namespace OriGine;

// ==========================================
// ChangeEditEntityCommand
// ==========================================

ChangeEditEntityCommand::ChangeEditEntityCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _toHandle, OriGine::EntityHandle _fromHandle)
    : inspectorArea_(_inspectorArea), toHandle_(_toHandle), fromHandle_(_fromHandle) {
    Scene* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();

    if (!currentScene) {
        return;
    }

    SceneFactory factory;
    if (toHandle_.IsValid()) {
        Entity* toEntity = currentScene->GetEntityRepositoryRef()->GetEntity(toHandle_);
        toEntityData_    = factory.CreateEntityJsonFromEntity(currentScene, toEntity);
    }
    if (fromHandle_.IsValid()) {
        Entity* fromEntity = currentScene->GetEntityRepositoryRef()->GetEntity(fromHandle_);
        fromEntityData_    = factory.CreateEntityJsonFromEntity(currentScene, fromEntity);
    }
}

void ChangeEditEntityCommand::Execute() {
    inspectorArea_->SetEditEntityHandle(toHandle_);
    inspectorArea_->ClearSystemMap();

    if (!toHandle_.IsValid()) {
        return;
    }
    Scene* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* toEntity    = currentScene->GetEntityRepositoryRef()->GetEntity(toHandle_);
    inspectorArea_->SetEditEntityName(toEntity->GetDataType());

    if (toEntityData_.empty()) {
        LOG_DEBUG("ChangeEditEntityCommand::Execute: toEntityData is empty, skipping entity change.");
        return;
    }

    /// Systems の読み込み
    auto& entitySystemData = toEntityData_.at("Systems");
    auto& systemMap        = currentScene->GetSystemRunnerRef()->GetSystems();
    for (auto& systemData : entitySystemData.items()) {
        const ::std::string& systemName = systemData.value().at("SystemName");
        auto systemItr                  = systemMap.find(systemName);
        if (systemItr == systemMap.end()) {
            LOG_ERROR("System '{}' not found .", systemName);
            continue;
        }
        std::shared_ptr<OriGine::ISystem> system = systemItr->second;
        if (!system) {
            LOG_ERROR("System '{}' not found for entity ID '{}'.", systemName, uuids::to_string(toHandle_.uuid));
            continue;
        }

        inspectorArea_->GetSystemMap()[int32_t(system->GetCategory())][systemName] = system;
    }
}

void ChangeEditEntityCommand::Undo() {
    inspectorArea_->SetEditEntityHandle(fromHandle_);
    inspectorArea_->ClearSystemMap();

    Scene* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* fromEntity  = currentScene->GetEntityRepositoryRef()->GetEntity(fromHandle_);
    if (!fromEntity) {
        LOG_DEBUG("FromEntity is nullptr.");
        return;
    }
    inspectorArea_->SetEditEntityName(fromEntity->GetDataType());

    if (fromEntityData_.empty()) {
        LOG_DEBUG("fromEntityData is empty, skipping entity change.");
        return;
    }

    /// Systems の読み込み
    auto& entitySystemData = fromEntityData_.at("Systems");
    auto& systemMap        = currentScene->GetSystemRunnerRef()->GetSystems();
    for (auto& systemData : entitySystemData.items()) {
        const ::std::string& systemName = systemData.value().at("SystemName");
        auto systemItr                  = systemMap.find(systemName);
        if (systemItr == systemMap.end()) {
            LOG_ERROR("System '{}' not found .", systemName);
            continue;
        }
        std::shared_ptr<OriGine::ISystem> system = systemItr->second;
        if (!system) {
            LOG_ERROR("System '{}' not found for entity ID '{}'.", systemName, uuids::to_string(toHandle_.uuid));
            continue;
        }
        inspectorArea_->GetSystemMap()[int32_t(system->GetCategory())][systemName] = system;
    }
}

// ==========================================
// ChangeEntityUniquenessCommand
// ==========================================

void ChangeEntityUniquenessCommand::Execute() {
    auto* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity     = currentScene->GetEntity(entityHandle_);

    if (!entity) {
        LOG_ERROR("Entity not found. \n Handle '{}' \n", uuids::to_string(entityHandle_.uuid));
        return;
    }

    if (newValue_) {
        currentScene->RegisterUniqueEntity(entity);
    } else {
        currentScene->UnregisterUniqueEntity(entity);
    }
}

void ChangeEntityUniquenessCommand::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntity(entityHandle_);

    if (!entity) {
        LOG_ERROR("Entity not found. \n  Handle : {} \n", uuids::to_string(entityHandle_.uuid));
        return;
    }

    if (oldValue_) {
        currentScene->RegisterUniqueEntity(entity);
    } else {
        currentScene->UnregisterUniqueEntity(entity);
    }
}

// ==========================================
// ChangeEntityShouldSaveCommand
// ==========================================

void ChangeEntityShouldSaveCommand::Execute() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntity(entityHandle_);

    if (!entity) {
        LOG_ERROR("Entity not found. \n  Handle : {} \n", uuids::to_string(entityHandle_.uuid));
        return;
    }

    entity->SetShouldSave(newValue_);
}

void ChangeEntityShouldSaveCommand::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntity(entityHandle_);

    if (!entity) {
        LOG_ERROR("Entity not found. \n  Handle : {} \n", uuids::to_string(entityHandle_.uuid));
        return;
    }

    entity->SetShouldSave(oldValue_);
}

// ==========================================
// ChangeEntityNameCommand
// ==========================================

ChangeEntityNameCommand::ChangeEntityNameCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _entityHandle, const ::std::string& _newName)
    : inspectorArea_(_inspectorArea), entityHandle_(_entityHandle), newName_(_newName) {
    auto* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    oldName_           = currentScene->GetEntity(entityHandle_)->GetDataType();
}

void ChangeEntityNameCommand::Execute() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Execute: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }

    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->IsUnique()) {
        currentScene->UnregisterUniqueEntity(entity);
    }

    entity->SetDataType(newName_);

    if (entity->IsUnique()) {
        currentScene->RegisterUniqueEntity(entity);
    }
}

void ChangeEntityNameCommand::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Undo: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }

    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->IsUnique()) {
        currentScene->UnregisterUniqueEntity(entity);
    }

    entity->SetDataType(oldName_);

    if (entity->IsUnique()) {
        currentScene->RegisterUniqueEntity(entity);
    }
}

// ==========================================
// DeleteEntityCommand
// ==========================================

DeleteEntityCommand::DeleteEntityCommand(EntityInspectorArea* _parentArea, EntityHandle _entityHandle)
    : parentArea_(_parentArea), entityHandle_(_entityHandle) {}

void DeleteEntityCommand::Execute() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Execute: No current scene found.");
        return;
    }
    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Execute: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }

    SceneFactory factory;

    entityData_ = factory.CreateEntityJsonFromEntity(currentScene, entity);
    currentScene->AddDeleteEntity(entityHandle_);
    LOG_DEBUG("DeleteEntityCommand::Execute: Deleted entity with ID '{}'.", uuids::to_string(entityHandle_.uuid));
}

void DeleteEntityCommand::Undo() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Undo: No current scene found.");
        return;
    }

    SceneFactory factory;
    // エンティティを復元
    Entity* entity = factory.BuildEntity(currentScene, entityData_);
    entityHandle_  = entity->GetHandle(); // 復元後のエンティティIDを更新
    // 編集対象エンティティを復元したエンティティに変更
    ChangeEditEntityCommand changeEditEntity = ChangeEditEntityCommand(parentArea_, entityHandle_, parentArea_->GetEditEntityHandle());
    changeEditEntity.Execute();

    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Undo: Failed to restore entity with ID '{}'.", uuids::to_string(entityHandle_.uuid));
        return;
    }
    LOG_DEBUG("DeleteEntityCommand::Undo: Restored entity with ID '{}'.", uuids::to_string(entityHandle_.uuid));
}

// ==========================================
// RemoveComponentFromEditListCommand
// ==========================================

RemoveComponentFromEditListCommand::RemoveComponentFromEditListCommand(EntityInspectorArea* _parentArea, const ::std::string& _componentTypeName, int32_t _compIndex)
    : parentArea_(_parentArea), componentTypeName_(_componentTypeName), componentIndex_(_compIndex) {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    // 現在のコンポーネントデータを保存
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    const auto& compArray = scene->GetComponentArray(componentTypeName_);
    if (compArray) {
        EntityHandle editEntityHandle = parentArea_->GetEditEntityHandle();
        // コンポーネントデータを保存
        // 失敗したらエラーログを出す
        if (!compArray->SaveComponent(editEntityHandle, componentIndex_, componentData_)) {
            LOG_ERROR("RemoveComponentFromEditListCommand: Edit entity is null.");
        }
    } else {
        LOG_ERROR("RemoveComponentFromEditListCommand: Component array '{}' not found.", componentTypeName_);
    }
}

void RemoveComponentFromEditListCommand::Execute() {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    scene->RemoveComponent(componentTypeName_, parentArea_->GetEditEntityHandle(), componentIndex_);
}

void RemoveComponentFromEditListCommand::Undo() {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    EntityHandle editEntity = parentArea_->GetEditEntityHandle();
    auto componentArray     = scene->GetComponentArray(componentTypeName_);
    if (!componentArray) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Component array '{}' not found.", componentTypeName_);
        return;
    }

    componentArray->InsertComponent(scene, editEntity, componentIndex_);
    componentArray->LoadComponent(editEntity, componentIndex_, componentData_);
}

// ==========================================
// AddComponentTypeNamesCommand
// ==========================================

void AddComponentTypeNamesCommand::Execute() {
    parentArea_->AddComponentTypeName(componentTypeName_);
}

void AddComponentTypeNamesCommand::Undo() {
    parentArea_->RemoveComponentTypeName(componentTypeName_);
}

// ==========================================
// RemoveComponentTypeNamesCommand
// ==========================================

void RemoveComponentTypeNamesCommand::Execute() {
    parentArea_->RemoveComponentTypeName(componentTypeName_);
}

void RemoveComponentTypeNamesCommand::Undo() {
    parentArea_->AddComponentTypeName(componentTypeName_);
}

// ==========================================
// ClearComponentTypeNamesCommand
// ==========================================

void ClearComponentTypeNamesCommand::Execute() {
    componentTypeNames_ = parentArea_->GetComponentTypeNames();
    parentArea_->ClearComponentTypeNames();
    LOG_DEBUG("ClearComponentTypeNamesCommand::Execute: Cleared component type names.");
}

void ClearComponentTypeNamesCommand::Undo() {
    parentArea_->SetComponentTypeNames(componentTypeNames_);
    LOG_DEBUG("ClearComponentTypeNamesCommand::Undo: Restored component type names.");
}

// ==========================================
// SetComponentTargetEntitiesCommand
// ==========================================

SetComponentTargetEntitiesCommand::SetComponentTargetEntitiesCommand(SelectAddComponentArea* _parentArea, const ::std::list<OriGine::EntityHandle>& _targets)
    : parentArea_(_parentArea), targetEntityHandles_(_targets) {
    previousTargetEntityHandles_ = parentArea_->GetTargetEntityHandles();
}

void SetComponentTargetEntitiesCommand::Execute() {
    parentArea_->SetTargetEntityHandles(targetEntityHandles_);
}

void SetComponentTargetEntitiesCommand::Undo() {
    parentArea_->SetTargetEntityHandles(previousTargetEntityHandles_);
}

// ==========================================
// ClearComponentTargetEntitiesCommand
// ==========================================

ClearComponentTargetEntitiesCommand::ClearComponentTargetEntitiesCommand(SelectAddComponentArea* _parentArea)
    : parentArea_(_parentArea) {
    previousTargetEntityHandles_ = parentArea_->GetTargetEntityHandles();
}

void ClearComponentTargetEntitiesCommand::Execute() {
    parentArea_->ClearTarget();
}

void ClearComponentTargetEntitiesCommand::Undo() {
    parentArea_->SetTargetEntityHandles(previousTargetEntityHandles_);
}

// ==========================================
// AddSystemNamesCommand
// ==========================================

void AddSystemNamesCommand::Execute() {
    parentArea_->AddSystemTypeName(systemTypeName_);
    LOG_DEBUG("AddSystemNamesCommand::Execute: Added system type name '{}'.", systemTypeName_);
}

void AddSystemNamesCommand::Undo() {
    parentArea_->RemoveSystemTypeName(systemTypeName_);
    LOG_DEBUG("AddSystemNamesCommand::Undo: Removed system type name '{}'.", systemTypeName_);
}

// ==========================================
// RemoveSystemNamesCommand
// ==========================================

void RemoveSystemNamesCommand::Execute() {
    parentArea_->RemoveSystemTypeName(systemTypeName_);
    LOG_DEBUG("RemoveSystemNamesCommand::Execute: Removed system type name '{}'.", systemTypeName_);
}

void RemoveSystemNamesCommand::Undo() {
    parentArea_->AddSystemTypeName(systemTypeName_);
    LOG_DEBUG("RemoveSystemNamesCommand::Undo: Added system type name '{}' back.", systemTypeName_);
}

// ==========================================
// ClearSystemNamesCommand
// ==========================================

void ClearSystemNamesCommand::Execute() {
    systemTypeNames_ = parentArea_->GetSystemTypeNames();
    parentArea_->ClearSystemTypeNames();
    LOG_DEBUG("ClearSystemNamesCommand::Execute: Cleared system type names.");
}

void ClearSystemNamesCommand::Undo() {
    parentArea_->SetSystemTypeNames(systemTypeNames_);
    LOG_DEBUG("ClearSystemNamesCommand::Undo: Restored system type names.");
}

// ==========================================
// SetSystemTargetEntitiesCommand
// ==========================================

SetSystemTargetEntitiesCommand::SetSystemTargetEntitiesCommand(SelectAddSystemArea* _parentArea, const ::std::list<OriGine::EntityHandle>& _targets)
    : parentArea_(_parentArea), targetEntityHandles_(_targets) {
    previousTargetEntityHandles_ = parentArea_->GetTargetEntityHandles();
}

void SetSystemTargetEntitiesCommand::Execute() {
    parentArea_->SetTargetEntityHandles(targetEntityHandles_);
}

void SetSystemTargetEntitiesCommand::Undo() {
    parentArea_->SetTargetEntityHandles(previousTargetEntityHandles_);
}

// ==========================================
// ClearSystemTargetEntitiesCommand
// ==========================================

ClearSystemTargetEntitiesCommand::ClearSystemTargetEntitiesCommand(SelectAddSystemArea* _parentArea)
    : parentArea_(_parentArea) {
    previousTargetEntityHandles_ = parentArea_->GetTargetEntityHandles();
}

void ClearSystemTargetEntitiesCommand::Execute() {
    parentArea_->ClearTarget();
}

void ClearSystemTargetEntitiesCommand::Undo() {
    parentArea_->SetTargetEntityHandles(previousTargetEntityHandles_);
}

// ==========================================
// AddSystemsForTargetEntitiesCommand
// ==========================================

void AddSystemsForTargetEntitiesCommand::Execute() {
    auto window = parentArea_->GetParentWindow();
    if (!window) {
        LOG_ERROR("AddSystemsForTargetEntitiesCommand::Execute: No SceneEditorWindow found.");
        return;
    }
    auto currentScene = window->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntitiesCommand::Execute: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->GetArea(nameof<EntityInspectorArea>()).get());
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    EntityHandle editEntityHandle = entityInspectorArea->GetEditEntityHandle();
    for (const auto& entityId : targetEntityHandles_) {
        Entity* entity = currentScene->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", uuids::to_string(entityId.uuid));
            continue;
        }

        if (editEntityHandle == entityId) {
            for (const auto& systemTypeName : systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName, entityId);

                std::shared_ptr<OriGine::ISystem> system                                            = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName);
                entityInspectorArea->GetSystemMap()[int32_t(system->GetCategory())][systemTypeName] = system;
            }
        } else {
            for (const auto& systemTypeName : systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName, entityId);
            }
        }
    }
}

void AddSystemsForTargetEntitiesCommand::Undo() {
    auto window = parentArea_->GetParentWindow();
    if (!window) {
        LOG_ERROR("AddSystemsForTargetEntitiesCommand::Undo: No SceneEditorWindow found.");
        return;
    }
    auto currentScene = window->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntitiesCommand::Undo: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->GetArea(nameof<EntityInspectorArea>()).get());
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    EntityHandle editEntityHandle = entityInspectorArea->GetEditEntityHandle();
    for (const auto& entityId : targetEntityHandles_) {
        Entity* entity = currentScene->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", uuids::to_string(entityId.uuid));
            continue;
        }

        if (editEntityHandle == entityId) {
            for (const auto& systemTypeName : systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName, entityId);
                std::shared_ptr<OriGine::ISystem> system = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName);
                auto& systems                            = entityInspectorArea->GetSystemMap()[int32_t(system->GetCategory())];
                auto itr                                 = systems.find(systemTypeName);
                if (itr != systems.end()) {
                    systems.erase(itr);
                } else {
                    LOG_ERROR("System '{}' not found in EntityInspectorArea's system map.", systemTypeName);
                }
            }
        } else {
            for (const auto& systemTypeName : systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName, entityId);
            }
        }
    }
}

// ==========================================
// RemoveComponentForEntityCommand
// ==========================================

RemoveComponentForEntityCommand::RemoveComponentForEntityCommand(Scene* _scene, const ::std::string& _componentTypeName, EntityHandle _entityHandle, int32_t _compIndex)
    : scene_(_scene), componentTypeName_(_componentTypeName), entityHandle_(_entityHandle), compIndex_(_compIndex) {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand: Scene is null.");
        return;
    }

    if (entityHandle_.IsValid()) {
        Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityHandle_);
        if (!entity) {
            LOG_ERROR("RemoveComponentForEntityCommand: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
            return;
        }
        const auto& compArray = scene_->GetComponentArray(componentTypeName_);
        compArray->SaveComponent(entityHandle_, compIndex_, componentData_);
    }
}

void RemoveComponentForEntityCommand::Execute() {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Scene is null.");
        return;
    }
    Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }
    const auto& compArray = scene_->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Component array '{}' not found.", componentTypeName_);
        return;
    }
    compArray->RemoveComponent(entityHandle_, compIndex_);
}

void RemoveComponentForEntityCommand::Undo() {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Scene is null.");
        return;
    }
    Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Entity with ID '{}' not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }
    const auto& compArray = scene_->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Component array '{}' not found.", componentTypeName_);
        return;
    }
    compArray->InsertComponent(scene_, entityHandle_, compIndex_);
    compArray->LoadComponent(entityHandle_, compIndex_, componentData_);
}

#endif // _DEBUG
