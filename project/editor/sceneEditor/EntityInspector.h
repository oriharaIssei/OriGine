#pragma once

#ifdef _DEBUG

/// parent
#include "editor/IEditor.h"

/// ECS
// component
#include "component/ComponentArray.h"
#include "component/IComponent.h"
// system
#include "system/ISystem.h"

class SceneEditorWindow;

class EntityInspectorArea
    : public Editor::Area {
public:
    EntityInspectorArea(SceneEditorWindow* _window);
    ~EntityInspectorArea() override;
    void Initialize() override;
    // void DrawGui() override;
    void Finalize() override;

public:
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

    int32_t editEntityId_       = -1; // 編集中のエンティティID
    std::string editEntityName_ = ""; // 編集中のエンティティ名
    std::unordered_map<std::string, std::vector<IComponent*>> entityComponentMap_; // コンポーネントのマップ
    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)> systemMap_; // システムのマップ

public:
    SceneEditorWindow* getParentWindow() const {
        return parentWindow_;
    }
    int32_t getEditEntityId() const {
        return editEntityId_;
    }
    /// 変更は Commandを通して行うべき
    /*
    void setEditEntityId(int32_t id) {
        editEntityId_ = id;
    }*/

    const std::string& getEditEntityName() const {
        return editEntityName_;
    }
    std::string& getEditEntityNameRef() {
        return editEntityName_;
    }

    std::unordered_map<std::string, std::vector<IComponent*>>& getEntityComponentMap() {
        return entityComponentMap_;
    }
    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)>& getSystemMap() {
        return systemMap_;
    }
};

class EntityInfomationRegion
    : public Editor::Region {
public:
    EntityInfomationRegion(EntityInspectorArea* _parent);
    ~EntityInfomationRegion() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
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
    class ChangeEntityName
        : public IEditCommand {
    public:
        ChangeEntityName(EntityInspectorArea* _inspectorArea, int32_t _entityID, const std::string& _newName);
        ~ChangeEntityName() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        int32_t entityId_                   = -1; // 対象のエンティティID
        std::string oldName_; // 変更前のエンティティ名
        std::string newName_; // 変更後のエンティティ名
    };

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

class EntityComponentRegion
    : public Editor::Region {
public:
    EntityComponentRegion(EntityInspectorArea* _parent);
    ~EntityComponentRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    EntityInspectorArea* parentArea_ = nullptr; // 親エリアへのポインタ
};

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

        std::string searchBuff_ = "";
    };

    /// ==========================================
    /// Command
    /// ==========================================
    class AddComponentTypeNames
        : public IEditCommand {
    public:
        AddComponentTypeNames(SelectAddComponentArea* _parentArea, const std::string& _componentTypeName)
            : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
        ~AddComponentTypeNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string componentTypeName_; // 追加するコンポーネントのタイプ名
    };
    class RemoveComponentTypeNames
        : public IEditCommand {
    public:
        RemoveComponentTypeNames(SelectAddComponentArea* _parentArea, const std::string& _componentTypeName)
            : parentArea_(_parentArea), componentTypeName_(_componentTypeName) {}
        ~RemoveComponentTypeNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string componentTypeName_; // 削除するコンポーネントのタイプ名
    };
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
        std::vector<std::string> componentTypeNames_;
    };

    class SetTargeEntities
        : public IEditCommand {
    public:
        SetTargeEntities(SelectAddComponentArea* _parentArea, const std::list<int32_t>& _targets)
            : parentArea_(_parentArea), targetEntityIds_(_targets) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~SetTargeEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };
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
        std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    std::list<int32_t> targetEntityIds_;
    std::vector<std::string> componentTypeNames_; // 追加可能なコンポーネントのタイプ名

public:
    void clearTarget() { targetEntityIds_.clear(); }
    void setTargets(const std::list<int32_t>& _targets);
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

        std::string searchBuff_ = "";
    };

    /// ==========================================
    /// Command
    /// ==========================================
    class AddSystemCategoryNames
        : public IEditCommand {
    public:
        AddSystemCategoryNames(SelectAddSystemArea* _parentArea, const std::string& _systemTypeName)
            : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
        ~AddSystemCategoryNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string systemTypeName_; // 追加するコンポーネントのタイプ名
    };
    class RemoveSystemCategoryNames
        : public IEditCommand {
    public:
        RemoveSystemCategoryNames(SelectAddSystemArea* _parentArea, const std::string& _systemTypeName)
            : parentArea_(_parentArea), systemTypeName_(_systemTypeName) {}
        ~RemoveSystemCategoryNames() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string systemTypeName_; // 削除するコンポーネントのタイプ名
    };
    class ClearSystemCategoryNames
        : public IEditCommand {
    public:
        ClearSystemCategoryNames(SelectAddSystemArea* _parentArea)
            : parentArea_(_parentArea) {}
        ~ClearSystemCategoryNames() override = default;

        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::vector<std::string> systemTypeNames_;
    };

    class SetTargeEntities
        : public IEditCommand {
    public:
        SetTargeEntities(SelectAddSystemArea* _parentArea, const std::list<int32_t>& _targets)
            : parentArea_(_parentArea), targetEntityIds_(_targets) {
            previousTargetEntityIds_ = parentArea_->targetEntityIds_; // 現在のターゲットエンティティIDを保存
        }
        ~SetTargeEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };
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
        std::list<int32_t> previousTargetEntityIds_; // 前の対象のエンティティIDリスト
    };

    class AddSystemsForTargetEntities
        : public IEditCommand {
    public:
        AddSystemsForTargetEntities(SelectAddSystemArea* _parentArea, const std::list<int32_t>& _targets, const std::vector<std::string>& _compTypeNames)
            : parentArea_(_parentArea), targetEntityIds_(_targets), systemTypeNames_(_compTypeNames) {}
        ~AddSystemsForTargetEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddSystemArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        std::vector<std::string> systemTypeNames_; // 追加するコンポーネントのタイプ名
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    std::list<int32_t> targetEntityIds_;
    std::vector<std::string> systemTypeNames_; // 追加可能なコンポーネントのタイプ名

public:
    void clearTarget() { targetEntityIds_.clear(); }
    void setTargets(const std::list<int32_t>& _targets);
};

#endif // _DEBUG
