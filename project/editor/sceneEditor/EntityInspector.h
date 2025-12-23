#pragma once

#ifdef _DEBUG

/// parent
#include "editor/IEditor.h"

/// stl
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

/// ECS
// component
#include "component/IComponent.h"
// system
#include "system/ISystem.h"
#include "system/SystemCategory.h"

/// externals
#include <nlohmann/json.hpp>

/// editor
class SceneEditorWindow;

/// <summary>
/// Entity の Inspectorエリア
/// </summary>
class EntityInspectorArea
    : public Editor::Area {
public:
    EntityInspectorArea(SceneEditorWindow* _window);
    ~EntityInspectorArea() override;
    void Initialize() override;
    // void DrawGui() override;
    void Finalize() override;

public:
    /// <summary>
    /// 編集中のエンティティを変更するコマンド
    /// </summary>
    class ChangeEditEntityCommand
        : public IEditCommand {
    public:
        ChangeEditEntityCommand(EntityInspectorArea* _inspectorArea, int32_t _to, int32_t _from);
        ~ChangeEditEntityCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

        int32_t toId_   = -1; // 変更後のエンティティID
        int32_t fromId_ = -1; // 元のエンティティID

        nlohmann::json toEntityData_; // 変更後のエンティティデータ
        nlohmann::json fromEntityData_; // 元のエンティティデータ
    };

protected:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    // 編集中のエンティティデータ
    int32_t editEntityId_       = -1; // 編集中のエンティティID
    ::std::string editEntityName_ = ""; // 編集中のエンティティ名
    ::std::unordered_map<::std::string, ::std::vector<OriGine::IComponent*>> entityComponentMap_; // コンポーネントのマップ
    ::std::array<::std::unordered_map<::std::string, std::shared_ptr<OriGine::ISystem>>, size_t(OriGine::SystemCategory::Count)> systemMap_; // システムのマップ

public:
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
    int32_t GetEditEntityId() const {
        return editEntityId_;
    }
    /// 変更は Commandを通して行うべき
    /*
    void SetEditEntityId(int32_t id) {
        editEntityId_ = id;
    }*/

    const ::std::string& GetEditEntityName() const {
        return editEntityName_;
    }
    ::std::string& GetEditEntityNameRef() {
        return editEntityName_;
    }

    ::std::unordered_map<::std::string, ::std::vector<OriGine::IComponent*>>& GetEntityComponentMap() {
        return entityComponentMap_;
    }
    ::std::array<::std::unordered_map<::std::string, std::shared_ptr<OriGine::ISystem>>, size_t(OriGine::SystemCategory::Count)>& GetSystemMap() {
        return systemMap_;
    }
};

/// <summary>
/// Entityの基本情報を表示・編集するエリア
/// </summary>
class EntityInformationRegion
    : public Editor::Region {
public:
    EntityInformationRegion(EntityInspectorArea* _parent);
    ~EntityInformationRegion() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    /// <summary>
    /// Entityのユニーク性を変更するコマンド
    /// </summary>
    class ChangeEntityUniqueness
        : public IEditCommand {
    public:
        ChangeEntityUniqueness(EntityInspectorArea* _inspectorArea, int32_t _entityID, bool _oldValue)
            : inspectorArea_(_inspectorArea), entityId_(_entityID), oldValue_(_oldValue), newValue_(!oldValue_) {}
        ~ChangeEntityUniqueness() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

        int32_t entityId_ = -1; // 対象のエンティティID

        bool oldValue_ = false; // 変更前のユニーク性の値
        bool newValue_ = true; // 変更後のユニーク性の値
    };
    /// <summary>
    /// Entityを保存対象にするか変更するコマンド
    /// </summary>
    class ChangeEntityShouldSave
        : public IEditCommand {
    public:
        ChangeEntityShouldSave(EntityInspectorArea* _inspectorArea, int32_t _entityID, bool _oldValue)
            : inspectorArea_(_inspectorArea), entityId_(_entityID), oldValue_(_oldValue), newValue_(!oldValue_) {}
        ~ChangeEntityShouldSave() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

        int32_t entityId_ = -1; // 対象のエンティティID

        bool oldValue_ = false;
        bool newValue_ = true;
    };
    /// <summary>
    /// Entityの名前を変更するコマンド
    /// </summary>
    class ChangeEntityName
        : public IEditCommand {
    public:
        ChangeEntityName(EntityInspectorArea* _inspectorArea, int32_t _entityID, const ::std::string& _newName);
        ~ChangeEntityName() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        int32_t entityId_                   = -1; // 対象のエンティティID
        ::std::string oldName_; // 変更前のエンティティ名
        ::std::string newName_; // 変更後のエンティティ名
    };
    /// <summary>
    /// Entityを削除するコマンド
    /// </summary>
    class DeleteEntityCommand
        : public IEditCommand {
    public:
        DeleteEntityCommand(EntityInspectorArea* _parentArea, int32_t _entityId);
        ~DeleteEntityCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ
        int32_t entityId_; // 削除するエンティティのID
        nlohmann::json entityData_; // 削除するエンティティのデータ
    };

private:
    EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ
};

/// <summary>
/// EntityのComponentを表示・編集するエリア
/// </summary>
class EntityComponentRegion
    : public Editor::Region {
public:
    EntityComponentRegion(EntityInspectorArea* _parent);
    ~EntityComponentRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
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
        EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ

        ::std::string componentTypeName_; // 削除するコンポーネントのタイプ名
        int32_t componentIndex_ = 0; // 削除するコンポーネントのインデックス

        nlohmann::json componentData_; // 削除するコンポーネントのデータ
    };

private:
    EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ
};

/// <summary>
/// EntityのSystemを表示・編集するエリア
/// </summary>
class EntitySystemRegion
    : public Editor::Region {
public:
    EntitySystemRegion(EntityInspectorArea* _parent);
    ~EntitySystemRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
private:
    EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ
};

/// <summary>
/// Entityに追加する Componentを 選択するエリア
/// </summary>
class SelectAddComponentArea
    : public Editor::Area {
public:
    SelectAddComponentArea(SceneEditorWindow* _parentWindow);
    ~SelectAddComponentArea();

    void Initialize();
    void DrawGui();
    void Finalize();

protected:
    /// ==========================================
    /// Region
    /// ==========================================

    /// <summary>
    /// Entityに追加するComponentのリストを表示するRegion
    /// </summary>
    class ComponentListRegion
        : public Editor::Region {
    public:
        ComponentListRegion(SelectAddComponentArea* _parentArea);
        ~ComponentListRegion();

        void Initialize();
        void DrawGui();
        void Finalize();

    private:
        SelectAddComponentArea* parentArea_ = nullptr;

        ::std::string searchBuff_ = "";
    };

    /// ==========================================
    /// Command
    /// ==========================================

    /// <summary>
    /// 追加するComponentを追加するコマンド
    /// </summary>
    class AddComponentTypeNames
        : public IEditCommand {
    public:
        AddComponentTypeNames(SelectAddComponentArea* _parentArea, const ::std::string& _componentTypeName)
            : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
        ~AddComponentTypeNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string componentTypeName_; // 追加するコンポーネントのタイプ名
    };
    /// <summary>
    /// 追加するComponentを削除するコマンド
    /// </summary>
    class RemoveComponentTypeNames
        : public IEditCommand {
    public:
        RemoveComponentTypeNames(SelectAddComponentArea* _parentArea, const ::std::string& _componentTypeName)
            : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
        ~RemoveComponentTypeNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string componentTypeName_; // 削除するコンポーネントのタイプ名
    };
    /// <summary>
    /// 追加するComponentを全て取り消すコマンド
    /// </summary>
    class ClearComponentTypeNames
        : public IEditCommand {
    public:
        ClearComponentTypeNames(SelectAddComponentArea* _parentArea)
            : parentArea_(_parentArea) {}
        ~ClearComponentTypeNames() override = default;

        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::vector<::std::string> componentTypeNames_;
    };
    /// <summary>
    /// コンポーネントを追加する対象のエンティティを設定するコマンド
    /// </summary>
    class SetTargeEntities
        : public IEditCommand {
    public:
        SetTargeEntities(SelectAddComponentArea* _parentArea, const ::std::list<int32_t>& _targets)
            : parentArea_(_parentArea), targetEntityIds_(_targets) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~SetTargeEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        ::std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };
    /// <summary>
    /// コンポーネントを追加する対象のエンティティをクリアするコマンド
    /// </summary>
    class ClearTargetEntities
        : public IEditCommand {
    public:
        ClearTargetEntities(SelectAddComponentArea* _parentArea)
            : parentArea_(_parentArea) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~ClearTargetEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    ::std::list<int32_t> targetEntityIds_;
    ::std::vector<::std::string> componentTypeNames_; // 追加可能なコンポーネントのタイプ名

public:
    void ClearTarget() { targetEntityIds_.clear(); }
    void SetTargets(const ::std::list<int32_t>& _targets);
};

/// <summary>
/// Entityに追加する Systemを 選択するエリア
/// </summary>
class SelectAddSystemArea
    : public Editor::Area {
public:
    SelectAddSystemArea(SceneEditorWindow* _parentWindow);
    ~SelectAddSystemArea();

    void Initialize();
    void DrawGui();
    void Finalize();

public:
    /// ==========================================
    /// Region
    /// ==========================================

    /// <summary>
    /// Entityに追加するSystemのリストを表示するRegion
    /// </summary>
    class SystemListRegion
        : public Editor::Region {
    public:
        SystemListRegion(SelectAddSystemArea* _parentArea);
        ~SystemListRegion();

        void Initialize();
        void DrawGui();
        void Finalize();

    private:
        SelectAddSystemArea* parentArea_ = nullptr;

        ::std::string searchBuff_ = "";
    };

    /// ==========================================
    /// Command
    /// ==========================================

    /// <summary>
    /// Entityに追加するSystemを追加するコマンド
    /// </summary>
    class AddSystemNames
        : public IEditCommand {
    public:
        AddSystemNames(SelectAddSystemArea* _parentArea, const ::std::string& _systemTypeName)
            : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
        ~AddSystemNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string systemTypeName_; // 追加するコンポーネントのタイプ名
    };
    /// <summary>
    /// Entityに追加するSystemを削除するコマンド
    /// </summary>
    class RemoveSystemNames
        : public IEditCommand {
    public:
        RemoveSystemNames(SelectAddSystemArea* _parentArea, const ::std::string& _systemTypeName)
            : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
        ~RemoveSystemNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string systemTypeName_; // 削除するコンポーネントのタイプ名
    };
    /// <summary>
    /// Entityに追加するSystemを全て取り消すコマンド
    /// </summary>
    class ClearSystemNames
        : public IEditCommand {
    public:
        ClearSystemNames(SelectAddSystemArea* _parentArea)
            : parentArea_(_parentArea) {}
        ~ClearSystemNames() override = default;

        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::vector<::std::string> systemTypeNames_;
    };
    /// <summary>
    /// エンティティに追加するSystemの対象エンティティを設定するコマンド
    /// </summary>
    class SetTargeEntities
        : public IEditCommand {
    public:
        SetTargeEntities(SelectAddSystemArea* _parentArea, const ::std::list<int32_t>& _targets)
            : parentArea_(_parentArea), targetEntityIds_(_targets) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~SetTargeEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        ::std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };
    /// <summary>
    /// システムの追加対象のエンティティをクリアするコマンド
    /// </summary>
    class ClearTargetEntities
        : public IEditCommand {
    public:
        ClearTargetEntities(SelectAddSystemArea* _parentArea)
            : parentArea_(_parentArea) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~ClearTargetEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };
    /// <summary>
    /// システムを追加する対象のエンティティにシステムを追加するコマンド
    /// </summary>
    class AddSystemsForTargetEntities
        : public IEditCommand {
    public:
        AddSystemsForTargetEntities(SelectAddSystemArea* _parentArea, const ::std::list<int32_t>& _targets, const ::std::vector<::std::string>& _compTypeNames)
            : parentArea_(_parentArea), targetEntityIds_(_targets), systemTypeNames_(_compTypeNames) {}
        ~AddSystemsForTargetEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        ::std::vector<::std::string> systemTypeNames_; // 追加するコンポーネントのタイプ名
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    ::std::list<int32_t> targetEntityIds_;
    ::std::vector<::std::string> systemTypeNames_; // 追加可能なコンポーネントのタイプ名

public:
    void ClearTarget() { targetEntityIds_.clear(); }
    void SetTargets(const ::std::list<int32_t>& _targets);
};

/// <summary>
/// エンティティからコンポーネントを削除するコマンド
/// </summary>
class RemoveComponentForEntityCommand
    : public IEditCommand {
public:
    RemoveComponentForEntityCommand(OriGine::Scene* _scene, const ::std::string& _componentTypeName, int32_t _entityId, int32_t _compIndex = 0);
    ~RemoveComponentForEntityCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    OriGine::Scene* scene_ = nullptr; // 対象シーン
    ::std::string componentTypeName_; // 追加するコンポーネントのタイプ名
    int32_t entityId_  = -1; // 対象のエンティティID
    int32_t compIndex_ = 0; // 削除するコンポーネントのインデックス
    nlohmann::json componentData_; // 削除するコンポーネントのデータ
};

#endif // _DEBUG
