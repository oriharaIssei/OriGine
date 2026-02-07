#pragma once

#ifdef _DEBUG

/// parent
#include "editor/IEditor.h"

/// stl
#include <list>
#include <string>
#include <vector>

/// ECS
#include "component/IComponent.h"
#include "system/ISystem.h"
#include "system/SystemCategory.h"

/// externals
#include <nlohmann/json.hpp>

/// Forward declarations
class EntityInspectorArea;
class SelectAddComponentArea;
class SelectAddSystemArea;
class SceneEditorWindow;

namespace OriGine {
class Scene;
}

// ==========================================
// EntityInspectorArea Commands
// ==========================================

/// <summary>
/// 編集中のエンティティを変更するコマンド
/// </summary>
class ChangeEditEntityCommand
    : public IEditCommand {
public:
    ChangeEditEntityCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _toHandle, OriGine::EntityHandle _fromHandle);
    ~ChangeEditEntityCommand() override = default;

    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* inspectorArea_ = nullptr;
    OriGine::EntityHandle toHandle_     = OriGine::EntityHandle();
    OriGine::EntityHandle fromHandle_   = OriGine::EntityHandle();
    nlohmann::json toEntityData_;
    nlohmann::json fromEntityData_;
};

// ==========================================
// EntityInformationRegion Commands
// ==========================================

/// <summary>
/// Entityのユニーク性を変更するコマンド
/// </summary>
class ChangeEntityUniquenessCommand
    : public IEditCommand {
public:
    ChangeEntityUniquenessCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _entityHandle, bool _oldValue)
        : inspectorArea_(_inspectorArea), entityHandle_(_entityHandle), oldValue_(_oldValue), newValue_(!oldValue_) {}
    ~ChangeEntityUniquenessCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* inspectorArea_ = nullptr;
    OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle();
    bool oldValue_                      = false;
    bool newValue_                      = true;
};

/// <summary>
/// Entityを保存対象にするか変更するコマンド
/// </summary>
class ChangeEntityShouldSaveCommand
    : public IEditCommand {
public:
    ChangeEntityShouldSaveCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _entityHandle, bool _oldValue)
        : inspectorArea_(_inspectorArea), entityHandle_(_entityHandle), oldValue_(_oldValue), newValue_(!oldValue_) {}
    ~ChangeEntityShouldSaveCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* inspectorArea_ = nullptr;
    OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle();
    bool oldValue_                      = false;
    bool newValue_                      = true;
};

/// <summary>
/// Entityの名前を変更するコマンド
/// </summary>
class ChangeEntityNameCommand
    : public IEditCommand {
public:
    ChangeEntityNameCommand(EntityInspectorArea* _inspectorArea, OriGine::EntityHandle _entityHandle, const ::std::string& _newName);
    ~ChangeEntityNameCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* inspectorArea_ = nullptr;
    OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle();
    ::std::string oldName_;
    ::std::string newName_;
};

/// <summary>
/// Entityを削除するコマンド
/// </summary>
class DeleteEntityCommand
    : public IEditCommand {
public:
    DeleteEntityCommand(EntityInspectorArea* _parentArea, OriGine::EntityHandle _entityHandle);
    ~DeleteEntityCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* parentArea_ = nullptr;
    OriGine::EntityHandle entityHandle_;
    nlohmann::json entityData_;
};

// ==========================================
// EntityComponentRegion Commands
// ==========================================

/// <summary>
/// EntityのComponentを削除するコマンド
/// </summary>
class RemoveComponentFromEditListCommand
    : public IEditCommand {
public:
    RemoveComponentFromEditListCommand(EntityInspectorArea* _parentArea, const ::std::string& _componentTypeName, int32_t _compIndex);
    ~RemoveComponentFromEditListCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    EntityInspectorArea* parentArea_ = nullptr;
    ::std::string componentTypeName_;
    int32_t componentIndex_ = 0;
    nlohmann::json componentData_;
};

// ==========================================
// SelectAddComponentArea Commands
// ==========================================

/// <summary>
/// 追加するComponentを追加するコマンド
/// </summary>
class AddComponentTypeNamesCommand
    : public IEditCommand {
public:
    AddComponentTypeNamesCommand(SelectAddComponentArea* _parentArea, const ::std::string& _componentTypeName)
        : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
    ~AddComponentTypeNamesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddComponentArea* parentArea_ = nullptr;
    ::std::string componentTypeName_;
};

/// <summary>
/// 追加するComponentを削除するコマンド
/// </summary>
class RemoveComponentTypeNamesCommand
    : public IEditCommand {
public:
    RemoveComponentTypeNamesCommand(SelectAddComponentArea* _parentArea, const ::std::string& _componentTypeName)
        : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
    ~RemoveComponentTypeNamesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddComponentArea* parentArea_ = nullptr;
    ::std::string componentTypeName_;
};

/// <summary>
/// 追加するComponentを全て取り消すコマンド
/// </summary>
class ClearComponentTypeNamesCommand
    : public IEditCommand {
public:
    ClearComponentTypeNamesCommand(SelectAddComponentArea* _parentArea)
        : parentArea_(_parentArea) {}
    ~ClearComponentTypeNamesCommand() override = default;

    void Execute() override;
    void Undo() override;

private:
    SelectAddComponentArea* parentArea_ = nullptr;
    ::std::vector<::std::string> componentTypeNames_;
};

/// <summary>
/// コンポーネントを追加する対象のエンティティを設定するコマンド
/// </summary>
class SetComponentTargetEntitiesCommand
    : public IEditCommand {
public:
    SetComponentTargetEntitiesCommand(SelectAddComponentArea* _parentArea, const ::std::list<OriGine::EntityHandle>& _targets);
    ~SetComponentTargetEntitiesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddComponentArea* parentArea_ = nullptr;
    ::std::list<OriGine::EntityHandle> targetEntityHandles_;
    ::std::list<OriGine::EntityHandle> previousTargetEntityHandles_;
};

/// <summary>
/// コンポーネントを追加する対象のエンティティをクリアするコマンド
/// </summary>
class ClearComponentTargetEntitiesCommand
    : public IEditCommand {
public:
    ClearComponentTargetEntitiesCommand(SelectAddComponentArea* _parentArea);
    ~ClearComponentTargetEntitiesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddComponentArea* parentArea_ = nullptr;
    ::std::list<OriGine::EntityHandle> previousTargetEntityHandles_;
};

// ==========================================
// SelectAddSystemArea Commands
// ==========================================

/// <summary>
/// Entityに追加するSystemを追加するコマンド
/// </summary>
class AddSystemNamesCommand
    : public IEditCommand {
public:
    AddSystemNamesCommand(SelectAddSystemArea* _parentArea, const ::std::string& _systemTypeName)
        : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
    ~AddSystemNamesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::string systemTypeName_;
};

/// <summary>
/// Entityに追加するSystemを削除するコマンド
/// </summary>
class RemoveSystemNamesCommand
    : public IEditCommand {
public:
    RemoveSystemNamesCommand(SelectAddSystemArea* _parentArea, const ::std::string& _systemTypeName)
        : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
    ~RemoveSystemNamesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::string systemTypeName_;
};

/// <summary>
/// Entityに追加するSystemを全て取り消すコマンド
/// </summary>
class ClearSystemNamesCommand
    : public IEditCommand {
public:
    ClearSystemNamesCommand(SelectAddSystemArea* _parentArea)
        : parentArea_(_parentArea) {}
    ~ClearSystemNamesCommand() override = default;

    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::vector<::std::string> systemTypeNames_;
};

/// <summary>
/// エンティティに追加するSystemの対象エンティティを設定するコマンド
/// </summary>
class SetSystemTargetEntitiesCommand
    : public IEditCommand {
public:
    SetSystemTargetEntitiesCommand(SelectAddSystemArea* _parentArea, const ::std::list<OriGine::EntityHandle>& _targets);
    ~SetSystemTargetEntitiesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::list<OriGine::EntityHandle> targetEntityHandles_;
    ::std::list<OriGine::EntityHandle> previousTargetEntityHandles_;
};

/// <summary>
/// システムの追加対象のエンティティをクリアするコマンド
/// </summary>
class ClearSystemTargetEntitiesCommand
    : public IEditCommand {
public:
    ClearSystemTargetEntitiesCommand(SelectAddSystemArea* _parentArea);
    ~ClearSystemTargetEntitiesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::list<OriGine::EntityHandle> previousTargetEntityHandles_;
};

/// <summary>
/// システムを追加する対象のエンティティにシステムを追加するコマンド
/// </summary>
class AddSystemsForTargetEntitiesCommand
    : public IEditCommand {
public:
    AddSystemsForTargetEntitiesCommand(SelectAddSystemArea* _parentArea, const ::std::list<OriGine::EntityHandle>& _targets, const ::std::vector<::std::string>& _systemTypeNames)
        : parentArea_(_parentArea), targetEntityHandles_(_targets), systemTypeNames_(_systemTypeNames) {}
    ~AddSystemsForTargetEntitiesCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    SelectAddSystemArea* parentArea_ = nullptr;
    ::std::list<OriGine::EntityHandle> targetEntityHandles_;
    ::std::vector<::std::string> systemTypeNames_;
};

// ==========================================
// Standalone Commands
// ==========================================

/// <summary>
/// エンティティからコンポーネントを削除するコマンド
/// </summary>
class RemoveComponentForEntityCommand
    : public IEditCommand {
public:
    RemoveComponentForEntityCommand(OriGine::Scene* _scene, const ::std::string& _componentTypeName, OriGine::EntityHandle _entityHandle, int32_t _compIndex = 0);
    ~RemoveComponentForEntityCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    OriGine::Scene* scene_ = nullptr;
    ::std::string componentTypeName_;
    OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle();
    int32_t compIndex_                  = 0;
    nlohmann::json componentData_;
};

#endif // _DEBUG
