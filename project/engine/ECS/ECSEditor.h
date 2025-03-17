#pragma once

/// parent
#include "module/editor/IEditor.h"

/// stl
#include <memory>

/// ECS
#include "component/IComponent.h"
#include "ECSManager.h"
#include "Entity.h"
// util
#include "util/nameof.h"

class ECSEditor
    : public IEditor {
public:
    ECSEditor();
    ~ECSEditor();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

private:
    void SelectEntity();
    void EditEntity();
    void WorkerSystemList();

    /// <summary>
    ///  エンティティを WorkSystem に追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityJoinWorkSystem(GameEntity* _entity);
    /// <summary>
    /// エンティティに Component を追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityAddComponent(GameEntity* _entity);

    /// <summary>
    /// システムから離れる前のポップアップ(本当に抜けるのか確認)
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityLeaveWorkSystem(GameEntity* _entity);

private:
    ECSManager* ecsManager_ = nullptr;

    GameEntity* editEntity_ = nullptr;
    std::vector<std::pair<std::string, IComponent*>> editEntityComponents_;
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> editEntitySystems_;

    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> workSystemList_;

    // ポップアップの情報 (Name,ActiveState)
    struct ImGuiWindowPopUp {
        std::string name_;
        bool isOpen_ = false;
    };
    ImGuiWindowPopUp popupJoinWorkSystem_;
    ImGuiWindowPopUp popupAddComponent_;

    ImGuiWindowPopUp leaveWorkSystem_;
    std::string leaveSystemName_;
    ISystem* leaveSystem_ = nullptr;

public:
    void SortPriorityOrderFromECSManager();
    void SortPriorityOrderFromECSManager(int32_t systemTypeIndex);

    GameEntity* getEditEntity() const {
        return editEntity_;
    }
    void setEditEntity(GameEntity* _entity) {
        editEntity_ = _entity;
    }

    std::vector<std::pair<std::string, IComponent*>>& customEditComponents() {
        return editEntityComponents_;
    }
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)>& customEditEntitySystems() {
        return editEntitySystems_;
    }
};

#pragma region "Command"
/// <summary>
/// ECSEditor に対するコマンド (Base Class)
/// </summary>
class ECSEditorCommand
    : public IEditCommand {
public:
    ECSEditorCommand(ECSEditor* _ecsEditor)
        : ecsEditor_(_ecsEditor) {}

    ~ECSEditorCommand() {}

    virtual void Execute() = 0;
    virtual void Undo()    = 0;

protected:
    ECSEditor* ecsEditor_ = nullptr;
};

/// <summary>
/// エンティティを作成するコマンド
/// </summary>
class CreateEntityCommand
    : public ECSEditorCommand {
public:
    CreateEntityCommand(ECSEditor* _ecsEditor)
        : ECSEditorCommand(_ecsEditor) {}
    ~CreateEntityCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* createdEntity_ = nullptr;
};

/// <summary>
/// エンティティを削除するコマンド
/// </summary>
class EraseEntityCommand
    : public ECSEditorCommand {
public:
    EraseEntityCommand(ECSEditor* _ecsEditor, GameEntity* _erasedEntity)
        : ECSEditorCommand(_ecsEditor), erasedEntity_(_erasedEntity) {}

    ~EraseEntityCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* erasedEntity_ = nullptr;
    GameEntity erasedEntityBackup_;

    std::vector<std::pair<std::string, IComponent*>> erasedEntityComponents_;
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> erasedEntitySystems_;
};

/// <summary>
/// エンティティを選択するコマンド
/// </summary>
class SelectEntityCommand
    : public ECSEditorCommand {
public:
    SelectEntityCommand(ECSEditor* _ecsEditor, GameEntity* _selectedEntity)
        : ECSEditorCommand(_ecsEditor), nextEntity_(_selectedEntity) {
        preEntity_ = ecsEditor_->getEditEntity();
    }

    ~SelectEntityCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* preEntity_  = nullptr;
    GameEntity* nextEntity_ = nullptr;
};

/// <summary>
/// エンティティに Component を追加するコマンド
/// </summary>
class AddComponentCommand
    : public ECSEditorCommand {
public:
    AddComponentCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _componentTypeName)
        : ECSEditorCommand(_ecsEditor), entity_(_entity), componentTypeName_(_componentTypeName), addedComponentArray_(nullptr) {}
    ~AddComponentCommand() {}
    void Execute() override;
    void Undo() override;

private:
    GameEntity* entity_ = nullptr;
    std::string componentTypeName_;
    // Componentを追加された Array
    IComponentArray* addedComponentArray_;
    // 追加された ComponentのIndex
    int32_t addedComponentIndex_ = -1;
};

/// <summary>
/// エンティティから Component を削除するコマンド
/// </summary>
class RemoveComponentCommand
    : public ECSEditorCommand {
public:
    RemoveComponentCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _componentTypeName, int32_t _componentIndex)
        : ECSEditorCommand(_ecsEditor), entity_(_entity), componentTypeName_(_componentTypeName), componentIndex_(_componentIndex) {}
    ~RemoveComponentCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* entity_ = nullptr;

    std::string componentTypeName_;
    int32_t componentIndex_ = -1;
};

/// <summary>
/// エンティティをワークシステムに参加させるコマンド
/// </summary>
class JoinWorkSystemCommand
    : public ECSEditorCommand {
public:
    JoinWorkSystemCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _systemName, ISystem* _system)
        : ECSEditorCommand(_ecsEditor), entity_(_entity), systemName_(_systemName), system_(_system) {}
    ~JoinWorkSystemCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* entity_ = nullptr;
    std::string systemName_;
    int32_t typeIdx_ = -1;
    ISystem* system_ = nullptr;
};

/// <summary>
/// エンティティをワークシステムから離脱させるコマンド
/// </summary>
class LeaveWorkSystemCommand
    : public ECSEditorCommand {
public:
    LeaveWorkSystemCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _systemName, ISystem* _system)
        : ECSEditorCommand(_ecsEditor), entity_(_entity), systemName_(_systemName), system_(_system) {}
    ~LeaveWorkSystemCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* entity_ = nullptr;
    int32_t typeIdx_    = -1;
    std::string systemName_;
    ISystem* system_ = nullptr;
};

#pragma endregion
