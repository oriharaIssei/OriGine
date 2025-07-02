#pragma once

#include "IEditor.h"

/// stl
#include <memory>
#include <string>

/// engine
#include "component/IComponent.h"
#include "scene/Scene.h"
#include "system/ISystem.h"
enum class SystemCategory;
// camera
class DebugCamera;
/// lib
#include "globalVariables/SerializedField.h"

// util
#include "util/EnumBitMask.h"
#include "util/nameof.h"

class SceneEditorWindow
    : public Editor::Window {
public:
    SceneEditorWindow() : Editor::Window(nameof<SceneEditorWindow>()) {}
    ~SceneEditorWindow() {}
    void Initialize() override;
    void Finalize() override;

private:
    std::unique_ptr<Scene> currentScene_; // 現在のシーン
    SerializedField<std::string> editSceneName_ = SerializedField<std::string>("Settings", "SceneEditor", "editSceneName", "Game"); // 編集中のシーン名(保存する)
public:
    Scene* getCurrentScene() {
        return currentScene_.get();
    }
    void changeScene(std::unique_ptr<Scene>&& _newScene) {
        currentScene_.reset();
        currentScene_ = std::move(_newScene);
    }
    SerializedField<std::string>& getEditSceneName() {
        return editSceneName_;
    }
};

#pragma region "Menus"

/// <summary>
/// File Menu
/// </summary>
class FileMenu
    : public Editor::Menu {
public:
    FileMenu(SceneEditorWindow* _parentWindow);
    ~FileMenu() override;
    void Initialize() override;
    void Finalize() override;

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ
public:
    SceneEditorWindow* getParentWindow() const {
        return parentWindow_;
    }
};
class SaveMenuItem
    : public Editor::MenuItem {
public:
    SaveMenuItem(FileMenu* _parent);
    ~SaveMenuItem() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    FileMenu* parentMenu_ = nullptr; // 親メニューへのポインタ
    Scene* saveScene_     = nullptr; // 保存するシーンへのポインタ
};
class LoadMenuItem
    : public Editor::MenuItem {
public:
    LoadMenuItem(FileMenu* _parent);
    ~LoadMenuItem();

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    FileMenu* parentMenu_ = nullptr; // 親メニューへのポインタ
    Scene* loadScene_     = nullptr; // 保存するシーンへのポインタ
};
class CreateMenuItem
    : public Editor::MenuItem {
public:
    CreateMenuItem(FileMenu* _parent);
    ~CreateMenuItem();

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    FileMenu* parentMenu_     = nullptr; // 親メニューへのポインタ
    std::string newSceneName_ = "";
};

#pragma endregion

class SceneViewArea
    : public Editor::Area {
public:
    SceneViewArea(SceneEditorWindow* _parentWindow);
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    void DrawScene();

private:
    SceneEditorWindow* parentWindow_; // 親ウィンドウへのポインタ

    std::unique_ptr<DebugCamera> debugCamera_; // デバッグカメラ
};

#pragma region "EntityInspector"
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

    class AddComponentCommand
        : public IEditCommand {
    public:
        AddComponentCommand(EntityInspectorArea* _inspector, const std::string& _compTypeName)
            : inspectorArea_(_inspector), componentTypeName_(_compTypeName) {
        }
        ~AddComponentCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

        //nlohmann::json componentData_; // 追加するコンポーネントのデータ
        std::string componentTypeName_; // 追加するコンポーネントのタイプ名
    };
    class RemoveComponentCommand
        : public IEditCommand {
    public:
        RemoveComponentCommand(EntityInspectorArea* _inspector, const std::string& _compTypeName, int32_t _compIndex)
            : inspectorArea_(_inspector), componentTypeName_(_compTypeName) {}
        ~RemoveComponentCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

       // nlohmann::json componentData_; // 追加するコンポーネントのデータ
        int32_t componentIndex_ = -1; // 削除するコンポーネントのインデックス
        std::string componentTypeName_; // 削除するコンポーネントのタイプ名
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
    /*void setEditEntityId(int32_t id) {
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
        ChangeEntityName(EntityInspectorArea* _inspectorArea, int32_t _entityID, const std::string& _newName)
            : inspectorArea_(_inspectorArea), entityId_(_entityID), newName_(_newName) {
            auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
            oldName_          = currentScene->getEntityRepositoryRef()->getEntity(entityId_)->getDataType();
        }
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
    class AddSystemCommand
        : public IEditCommand {
    public:
        AddSystemCommand(EntityInspectorArea* _inspectorArea, const std::string& _systemTypeName, SystemCategory _category);
        ~AddSystemCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ

        SystemCategory systemCategory_ = SystemCategory::Initialize; // システムのカテゴリ
        std::string systemTypeName_; // 追加するシステムのタイプ名
    };

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

    class AddComponentsForTargetEntities
        : public IEditCommand {
    public:
        AddComponentsForTargetEntities(SelectAddComponentArea* _parentArea, const std::list<int32_t>& _targets)
            : parentArea_(_parentArea), targetEntityIds_(_targets) {}
        ~AddComponentsForTargetEntities() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SelectAddComponentArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> targetEntityIds_; // 対象のエンティティIDリスト
        //  std::vector<std::string> componentTypeNames_; // 追加するコンポーネントのタイプ名
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    std::list<int32_t> targetEntityIds_;
    std::vector<std::string> componentTypeNames_; // 追加可能なコンポーネントのタイプ名

public:
    void clearTarget() { targetEntityIds_.clear(); }
    void setTargets(const std::list<int32_t>& _targets);
};

#pragma endregion

class SystemInspectorArea
    : public Editor::Area {
public:
    SystemInspectorArea(SceneEditorWindow* _window);
    ~SystemInspectorArea() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    class ChangeSystemPriority
        : public IEditCommand {
    public:
        ChangeSystemPriority(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _oldPriority, int32_t _newPriority);
        ~ChangeSystemPriority() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string systemName_; // 対象のシステム名
        int32_t oldPriority_ = 0; // 変更前の優先度
        int32_t newPriority_ = 0; // 変更後の優先度
    };
    class ChangeSystemActivity
        : public IEditCommand {
    public:
        ChangeSystemActivity(SystemInspectorArea* _inspectorArea, const std::string& _systemName, bool _oldActivity, bool _newActivity);
        ~ChangeSystemActivity() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string systemName_; // 対象のシステム名
        bool oldActivity_ = false; // 変更前のアクティビティ状態
        bool newActivity_ = true; // 変更後のアクティビティ状態
    };

    enum class FilterType : int32_t {
        NONE                     = 0b1 << 0, // 全てのシステム
        ACTIVE                   = 0b1 << 1, // アクティブなシステム
        INACTIVE                 = 0b1 << 2, // 非アクティブなシステム
        CATEGORY_INITIALIZE      = 0b1 << 3,
        CATEGORY_INPUT           = 0b1 << 4,
        CATEGORY_STATETRANSITION = 0b1 << 5,
        CATEGORY_MOVEMENT        = 0b1 << 6,
        CATEGORY_COLLISION       = 0b1 << 7,
        CATEGORY_EFFECT          = 0b1 << 8,
        CATEGORY_RENER           = 0b1 << 9,
        CATEGORY_POSTRENDER      = 0b1 << 10,
        CATEGORY_ALL             = CATEGORY_INITIALIZE | CATEGORY_INPUT | CATEGORY_STATETRANSITION | CATEGORY_MOVEMENT | CATEGORY_COLLISION | CATEGORY_EFFECT | CATEGORY_RENER | CATEGORY_POSTRENDER,
        SEARCH                   = 0b1 << 11, // 検索フィルター
    };
    class ChangeSystemFilter
        : public IEditCommand {
    public:
        ChangeSystemFilter(SystemInspectorArea* _inspectorArea, int32_t _newFilter)
            : inspectorArea_(_inspectorArea), newFilter_(_newFilter) {
            oldFilter_ = static_cast<int32_t>(inspectorArea_->filter_);
        }
        ~ChangeSystemFilter() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        int32_t oldFilter_; // 変更前のフィルター
        int32_t newFilter_; // 変更後のフィルター
    };
    class ChangeSearchFilter
        : public IEditCommand {
    public:
        ChangeSearchFilter(SystemInspectorArea* _inspectorArea, const std::string& _oldBuffer)
            : inspectorArea_(_inspectorArea), oldSearchBuffer_(_oldBuffer) {
            newSearchBuffer_ = inspectorArea_->searchBuffer_;
        }
        ~ChangeSearchFilter() override = default;
        void Execute() override;
        void Undo() override;

    private:
        SystemInspectorArea* inspectorArea_ = nullptr; // 親エリアへのポインタ
        std::string oldSearchBuffer_; // 変更前の検索バッファ
        std::string newSearchBuffer_; // 変更後の検索バッファ
    };

protected:
    void SystemGui(const std::string& _systemName, ISystem* _system); // システムのGUIを描画

protected:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ
    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)> systemMap_; // システムのマップ

    const int32_t searchBufferSize_ = 256; // 検索バッファのサイズ
    std::string searchBuffer_; // 検索用のバッファ
    EnumBitmask<FilterType> filter_ = EnumBitmask<FilterType>(FilterType::CATEGORY_ALL); // フィルターの種類
public:
    SceneEditorWindow* getParentWindow() const {
        return parentWindow_;
    }
};

class HierarchyArea
    : public Editor::Area {
public:
    HierarchyArea(SceneEditorWindow* _window);
    ~HierarchyArea() override;

    void Initialize() override;
    // void DrawGui() override;
    void Finalize() override;

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ
public:
    SceneEditorWindow* getParentWindow() const {
        return parentWindow_;
    }
};

class EntityHierarchy
    : public Editor::Region {
public:
    EntityHierarchy(HierarchyArea* _parent);
    ~EntityHierarchy() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    class AddSelectedEntitiesCommand
        : public IEditCommand {
    public:
        AddSelectedEntitiesCommand(EntityHierarchy* _hierarchy, int32_t _addedEntityId);
        ~AddSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        int32_t addedEntityId_      = -1; // 追加されたエンティティID
    };
    class RemoveSelectedEntitiesCommand
        : public IEditCommand {
    public:
        RemoveSelectedEntitiesCommand(EntityHierarchy* _hierarchy, int32_t _removedEntityId);
        ~RemoveSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        int32_t removedEntityId_    = -1; // 削除されたエンティティID
    };
    class ClearSelectedEntitiesCommand
        : public IEditCommand {
    public:
        ClearSelectedEntitiesCommand(EntityHierarchy* _hierarchy);
        ~ClearSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> previousSelectedEntityIds_; // 以前の選択されたエンティティIDのリスト
    };

    class CreateEntityCommand
        : public IEditCommand {
    public:
        CreateEntityCommand(HierarchyArea* _parentArea, const std::string& _entityName);
        ~CreateEntityCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string entityName_;
        int32_t entityId_ = -1; // 作成するエンティティの名前
        nlohmann::json entityData_; // 作成するエンティティのデータ
    };

private:
    HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
    std::list<int32_t> selectedEntityIds_; // 選択されているオブジェクトのIDリスト
public:
    const std::list<int32_t>& getSelectedEntityIds() const {
        return selectedEntityIds_;
    }
    std::list<int32_t>& getSelectedEntityIdsRef() {
        return selectedEntityIds_;
    }

    HierarchyArea* getParentArea() const {
        return parentArea_;
    }
};
