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

/// Forward declarations for commands
class ChangeEditEntityCommand;
class ChangeEntityUniquenessCommand;
class ChangeEntityShouldSaveCommand;
class ChangeEntityNameCommand;
class DeleteEntityCommand;
class RemoveComponentFromEditListCommand;
class AddComponentTypeNamesCommand;
class RemoveComponentTypeNamesCommand;
class ClearComponentTypeNamesCommand;
class SetComponentTargetEntitiesCommand;
class ClearComponentTargetEntitiesCommand;
class AddSystemNamesCommand;
class RemoveSystemNamesCommand;
class ClearSystemNamesCommand;
class SetSystemTargetEntitiesCommand;
class ClearSystemTargetEntitiesCommand;
class AddSystemsForTargetEntitiesCommand;

/// <summary>
/// Entity の Inspectorエリア
/// </summary>
class EntityInspectorArea
    : public Editor::Area {
    // コマンドクラスへのfriend宣言
    friend class ::ChangeEditEntityCommand;
    friend class ::ChangeEntityUniquenessCommand;
    friend class ::ChangeEntityShouldSaveCommand;
    friend class ::ChangeEntityNameCommand;
    friend class ::DeleteEntityCommand;
    friend class ::RemoveComponentFromEditListCommand;

public:
    EntityInspectorArea(SceneEditorWindow* _window);
    ~EntityInspectorArea() override;
    void Initialize() override;
    // void DrawGui() override;
    void Finalize() override;

protected:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    // 編集中のエンティティデータ
    OriGine::EntityHandle editEntityHandle_ = OriGine::EntityHandle(); // 編集中のエンティティID
    ::std::string editEntityName_           = ""; // 編集中のエンティティ名
    ::std::array<::std::unordered_map<::std::string, std::shared_ptr<OriGine::ISystem>>, size_t(OriGine::SystemCategory::Count)> systemMap_; // システムのマップ

public:
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
    OriGine::EntityHandle GetEditEntityHandle() const {
        return editEntityHandle_;
    }
    void SetEditEntityHandle(OriGine::EntityHandle _handle) {
        editEntityHandle_ = _handle;
    }

    const ::std::string& GetEditEntityName() const {
        return editEntityName_;
    }
    ::std::string& GetEditEntityNameRef() {
        return editEntityName_;
    }
    void SetEditEntityName(const ::std::string& _name) {
        editEntityName_ = _name;
    }

    ::std::array<::std::unordered_map<::std::string, std::shared_ptr<OriGine::ISystem>>, size_t(OriGine::SystemCategory::Count)>& GetSystemMap() {
        return systemMap_;
    }
    void ClearSystemMap() {
        systemMap_.fill({});
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
    // コマンドクラスへのfriend宣言
    friend class ::AddComponentTypeNamesCommand;
    friend class ::RemoveComponentTypeNamesCommand;
    friend class ::ClearComponentTypeNamesCommand;
    friend class ::SetComponentTargetEntitiesCommand;
    friend class ::ClearComponentTargetEntitiesCommand;

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
        ::std::string searchBuff_           = "";
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr;
    ::std::list<OriGine::EntityHandle> targetEntityHandles_;
    ::std::vector<::std::string> componentTypeNames_;

public:
    void ClearTarget() { targetEntityHandles_.clear(); }
    void SetTargets(const ::std::list<OriGine::EntityHandle>& _targets);

    // アクセサメソッド
    const ::std::list<OriGine::EntityHandle>& GetTargetEntityHandles() const { return targetEntityHandles_; }
    void SetTargetEntityHandles(const ::std::list<OriGine::EntityHandle>& _handles) { targetEntityHandles_ = _handles; }
    const ::std::vector<::std::string>& GetComponentTypeNames() const { return componentTypeNames_; }
    void SetComponentTypeNames(const ::std::vector<::std::string>& _names) { componentTypeNames_ = _names; }
    void AddComponentTypeName(const ::std::string& _name) { componentTypeNames_.push_back(_name); }
    void RemoveComponentTypeName(const ::std::string& _name);
    void ClearComponentTypeNames() { componentTypeNames_.clear(); }
};

/// <summary>
/// Entityに追加する Systemを 選択するエリア
/// </summary>
class SelectAddSystemArea
    : public Editor::Area {
    // コマンドクラスへのfriend宣言
    friend class ::AddSystemNamesCommand;
    friend class ::RemoveSystemNamesCommand;
    friend class ::ClearSystemNamesCommand;
    friend class ::SetSystemTargetEntitiesCommand;
    friend class ::ClearSystemTargetEntitiesCommand;
    friend class ::AddSystemsForTargetEntitiesCommand;

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
        ::std::string searchBuff_        = "";
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr;
    ::std::list<OriGine::EntityHandle> targetEntityHandles_;
    ::std::vector<::std::string> systemTypeNames_;

public:
    void ClearTarget() { targetEntityHandles_.clear(); }
    void SetTargets(const ::std::list<OriGine::EntityHandle>& _targets);

    // アクセサメソッド
    SceneEditorWindow* GetParentWindow() const { return parentWindow_; }
    const ::std::list<OriGine::EntityHandle>& GetTargetEntityHandles() const { return targetEntityHandles_; }
    void SetTargetEntityHandles(const ::std::list<OriGine::EntityHandle>& _handles) { targetEntityHandles_ = _handles; }
    const ::std::vector<::std::string>& GetSystemTypeNames() const { return systemTypeNames_; }
    void SetSystemTypeNames(const ::std::vector<::std::string>& _names) { systemTypeNames_ = _names; }
    void AddSystemTypeName(const ::std::string& _name) { systemTypeNames_.push_back(_name); }
    void RemoveSystemTypeName(const ::std::string& _name);
    void ClearSystemTypeNames() { systemTypeNames_.clear(); }
};

#endif // _DEBUG
