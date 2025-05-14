#pragma once

/// parent
#include "module/editor/IEditor.h"

/// stl
#include <deque>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

/// ECS
#include "component/IComponent.h"
#include "ECSManager.h"
#include "Entity.h"
// util
#include "util/nameof.h"

#ifdef _DEBUG

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

    ///  エンティティを WorkSystem に追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityJoinWorkSystem(GameEntity* _entity, bool _isGroup);
    /// <summary>
    /// エンティティに Component を追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityAddComponent(GameEntity* _entity, bool _isGroup);

    /// <summary>
    /// システムから離れる前のポップアップ(本当に抜けるのか確認)
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityLeaveWorkSystem(GameEntity* _entity, bool _isGroup);

private:
    ECSManager* ecsManager_ = nullptr;

    std::list<const GameEntity*> selectedEntities_;
    GameEntity* editEntity_ = nullptr;
    std::map<std::string, std::deque<IComponent*>> editEntityComponents_;
    std::array<std::map<std::string, ISystem*>, int32_t(SystemType::Count)> editEntitySystems_;

    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> workSystemList_;

    // ポップアップの情報 (Name,ActiveState)
    struct ImGuiWindowPopUp {
        std::string name_;
        bool isOpen_ = false;
    };
    ImGuiWindowPopUp popupJoinWorkSystem_;
    ImGuiWindowPopUp popupAddComponent_;

    ImGuiWindowPopUp popupLeaveWorkSystem_;
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

    std::map<std::string, std::deque<IComponent*>>& customEditComponents() {
        return editEntityComponents_;
    }
    std::array<std::map<std::string, ISystem*>, int32_t(SystemType::Count)>& customEditEntitySystems() {
        return editEntitySystems_;
    }
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)>& customWorkSystemList() {
        return workSystemList_;
    }

    // 選択されたエンティティを非 const ポインタとして取得するヘルパー
    std::vector<GameEntity*> getSelectedEntities() {
        std::vector<GameEntity*> entities;
        for (auto* e : selectedEntities_) {
            entities.push_back(const_cast<GameEntity*>(e));
        }
        return entities;
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
/// エンティティの DataType を変更するコマンド
/// </summary>
class ChangeEntityDataTypeCommand
    : public ECSEditorCommand {
public:
    ChangeEntityDataTypeCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _newDataType)
        : ECSEditorCommand(_ecsEditor), entity_(_entity), newDataType_(_newDataType) {
        oldDataType_ = entity_->getDataType();
    }

    ~ChangeEntityDataTypeCommand() {}

    void Execute() override;
    void Undo() override;

private:
    GameEntity* entity_ = nullptr;
    std::string oldDataType_;
    std::string newDataType_;
};

/// <summary>
/// エンティティに Component を追加するコマンド
/// </summary>
class AddComponentCommand
    : public ECSEditorCommand {
public:
    AddComponentCommand(ECSEditor* _ecsEditor, GameEntity* _entity, const std::string& _componentTypeName)
        : ECSEditorCommand(_ecsEditor),
          entity_(_entity),
          componentTypeName_(_componentTypeName),
          addedComponentArray_(nullptr) {
        entityIsEditEntity_ = (entity_ == ecsEditor_->getEditEntity());
    }
    ~AddComponentCommand() {}

    void Execute() override;
    void Undo() override;

private:
    bool entityIsEditEntity_ = false;

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
        : ECSEditorCommand(_ecsEditor),
          entity_(_entity),
          componentTypeName_(_componentTypeName),
          componentIndex_(_componentIndex) {
        entityIsEditEntity_ = (entity_ == ecsEditor_->getEditEntity());
    }
    ~RemoveComponentCommand() {}

    void Execute() override;
    void Undo() override;

private:
    bool entityIsEditEntity_ = false;

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
        : ECSEditorCommand(_ecsEditor), entity_(_entity), systemName_(_systemName), system_(_system) {
        entityIsEditEntity_ = (entity_ == ecsEditor_->getEditEntity());
    }
    ~JoinWorkSystemCommand() {}

    void Execute() override;
    void Undo() override;

private:
    bool entityIsEditEntity_ = false;

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
        : ECSEditorCommand(_ecsEditor), entity_(_entity), systemName_(_systemName), system_(_system) {
        entityIsEditEntity_ = (entity_ == ecsEditor_->getEditEntity());
    }
    ~LeaveWorkSystemCommand() {}

    void Execute() override;
    void Undo() override;

private:
    bool entityIsEditEntity_ = false;

    GameEntity* entity_ = nullptr;
    int32_t typeIdx_    = -1;
    std::string systemName_;
    ISystem* system_ = nullptr;
};

class ChangingSystemActivityCommand
    : public ECSEditorCommand {
public:
    ChangingSystemActivityCommand(ECSEditor* _ecsEditor, const std::string& _systemName, ISystem* _system, bool _isActive)
        : ECSEditorCommand(_ecsEditor), system_(_system), systemName_(_systemName), isActive_(_isActive) {}
    ~ChangingSystemActivityCommand() {}
    void Execute() override;
    void Undo() override;

private:
    ISystem* system_ = nullptr;
    std::string systemName_;
    bool isActive_ = false;
    std::vector<GameEntity*> entities_;
};
class ChangingSystemPriorityCommand
    : public ECSEditorCommand {
public:
    ChangingSystemPriorityCommand(ECSEditor* _ecsEditor, ISystem* _system, int32_t _newPriority)
        : ECSEditorCommand(_ecsEditor), system_(_system), newPriority_(_newPriority) {
        oldPriority_ = system_->getPriority();
    }
    ~ChangingSystemPriorityCommand() {}
    void Execute() override;
    void Undo() override;

private:
    ISystem* system_     = nullptr;
    int32_t newPriority_ = 0;
    int32_t oldPriority_ = 0;
};

#pragma endregion

#pragma region "Group Command"

// 新しい GroupCommand の基本クラスはそのまま
class ECSGroupCommand
    : public ECSEditorCommand {
public:
    ECSGroupCommand(ECSEditor* _ecsEditor) : ECSEditorCommand(_ecsEditor) {
        entities_ = ecsEditor_->getSelectedEntities();
    }
    virtual ~ECSGroupCommand() {}
    void Execute() override {
        for (auto* entity : entities_) {
            ExecuteForEntity(entity);
        }
    }
    void Undo() override {
        for (auto* entity : entities_) {
            UndoForEntity(entity);
        }
    }

protected:
    virtual void ExecuteForEntity(GameEntity* entity) = 0;
    virtual void UndoForEntity(GameEntity* entity)    = 0;

    std::vector<GameEntity*> entities_;
};

class GroupEraseEntityCommand : public ECSGroupCommand {
public:
    GroupEraseEntityCommand(ECSEditor* _ecsEditor)
        : ECSGroupCommand(_ecsEditor) {}

protected:
    void ExecuteForEntity(GameEntity* entity) override {
        auto cmd = std::make_unique<EraseEntityCommand>(ecsEditor_, entity);
        cmd->Execute();
        childCommands_.push_back(std::move(cmd));
    }
    void UndoForEntity(GameEntity* /*entity*/) override {
        childCommands_[undoIndex_++]->Undo();
    }

private:
    std::vector<std::unique_ptr<IEditCommand>> childCommands_;
    size_t undoIndex_ = 0;
};

class GroupAddComponentCommand : public ECSGroupCommand {
public:
    GroupAddComponentCommand(ECSEditor* _ecsEditor, const std::string& _componentTypeName)
        : ECSGroupCommand(_ecsEditor), componentTypeName_(_componentTypeName), undoIndex_(0) {}

protected:
    void ExecuteForEntity(GameEntity* entity) override {
        auto cmd = std::make_unique<AddComponentCommand>(ecsEditor_, entity, componentTypeName_);
        cmd->Execute();
        childCommands_.push_back(std::move(cmd));
    }
    void UndoForEntity(GameEntity* /*entity*/) override {
        childCommands_[undoIndex_++]->Undo();
    }

private:
    std::string componentTypeName_;
    std::vector<std::unique_ptr<IEditCommand>> childCommands_;
    size_t undoIndex_;
};

class GroupRemoveComponentCommand : public ECSGroupCommand {
public:
    GroupRemoveComponentCommand(ECSEditor* _ecsEditor, const std::string& _componentTypeName, int32_t _componentIndex)
        : ECSGroupCommand(_ecsEditor), componentTypeName_(_componentTypeName), componentIndex_(_componentIndex), undoIndex_(0) {}

protected:
    void ExecuteForEntity(GameEntity* entity) override {
        auto cmd = std::make_unique<RemoveComponentCommand>(ecsEditor_, entity, componentTypeName_, componentIndex_);
        cmd->Execute();
        childCommands_.push_back(std::move(cmd));
    }
    void UndoForEntity(GameEntity* /*entity*/) override {
        childCommands_[undoIndex_++]->Undo();
    }

private:
    std::string componentTypeName_;
    int32_t componentIndex_;
    std::vector<std::unique_ptr<IEditCommand>> childCommands_;
    size_t undoIndex_;
};

class GroupJoinWorkSystemCommand : public ECSGroupCommand {
public:
    GroupJoinWorkSystemCommand(ECSEditor* _ecsEditor, const std::string& _systemName, ISystem* _system)
        : ECSGroupCommand(_ecsEditor), systemName_(_systemName), system_(_system), undoIndex_(0) {}

protected:
    void ExecuteForEntity(GameEntity* entity) override {
        auto cmd = std::make_unique<JoinWorkSystemCommand>(ecsEditor_, entity, systemName_, system_);
        cmd->Execute();
        childCommands_.push_back(std::move(cmd));
    }
    void UndoForEntity(GameEntity* /*entity*/) override {
        childCommands_[undoIndex_++]->Undo();
    }

private:
    std::string systemName_;
    ISystem* system_;
    std::vector<std::unique_ptr<IEditCommand>> childCommands_;
    size_t undoIndex_;
};

class GroupLeaveWorkSystemCommand : public ECSGroupCommand {
public:
    GroupLeaveWorkSystemCommand(ECSEditor* _ecsEditor, const std::string& _systemName, ISystem* _system)
        : ECSGroupCommand(_ecsEditor), systemName_(_systemName), system_(_system), undoIndex_(0) {}

protected:
    void ExecuteForEntity(GameEntity* entity) override {
        auto cmd = std::make_unique<LeaveWorkSystemCommand>(ecsEditor_, entity, systemName_, system_);
        cmd->Execute();
        childCommands_.push_back(std::move(cmd));
    }
    void UndoForEntity(GameEntity* /*entity*/) override {
        childCommands_[undoIndex_++]->Undo();
    }

private:
    std::string systemName_;
    ISystem* system_;
    std::vector<std::unique_ptr<IEditCommand>> childCommands_;
    size_t undoIndex_;
};
#pragma endregion

#endif // _DEBUG
