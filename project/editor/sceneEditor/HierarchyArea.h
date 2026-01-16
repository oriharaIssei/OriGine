#pragma once

#ifdef _DEBUG

/// ECS
// entity
#include "entity/EntityHandle.h"

/// editor
#include "editor/IEditor.h"
#include "editor/sceneEditor/SceneEditor.h"

/// <summary>
/// Entityの一覧を表示・操作するエリア
/// </summary>
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
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
};

/// <summary>
/// Entityの一覧を表示・操作するリージョン(HierarchyAreaで動作する)
/// </summary>
class EntityHierarchyRegion
    : public Editor::Region {
public:
    EntityHierarchyRegion(HierarchyArea* _parent);
    ~EntityHierarchyRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

public:
    /// <summary>
    /// 編集するEntityを追加するコマンド
    /// </summary>
    class AddSelectedEntitiesCommand
        : public IEditCommand {
    public:
        AddSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy, OriGine::EntityHandle _addedEntityId);
        ~AddSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchyRegion* hierarchy_        = nullptr; // 親エリアへのポインタ
        OriGine::EntityHandle addedEntityHandle_ = OriGine::EntityHandle(); // 追加されたエンティティID
    };
    /// <summary>
    /// 編集するEntityを削除するコマンド
    /// </summary>
    class RemoveSelectedEntitiesCommand
        : public IEditCommand {
    public:
        RemoveSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy, OriGine::EntityHandle _removedEntityHandle);
        ~RemoveSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchyRegion* hierarchy_          = nullptr; // 親エリアへのポインタ
        OriGine::EntityHandle removedEntityHandle_ = OriGine::EntityHandle(); // 削除されたエンティティID
    };
    /// <summary>
    /// 編集するEntityの選択をクリアするコマンド
    /// </summary>
    class ClearSelectedEntitiesCommand
        : public IEditCommand {
    public:
        ClearSelectedEntitiesCommand(EntityHierarchyRegion* _hierarchy);
        ~ClearSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchyRegion* hierarchy_ = nullptr; // 親エリアへのポインタ
        ::std::list<OriGine::EntityHandle> previousselectedEntityHandles_; // 以前の選択されたエンティティIDのリスト
    };

    /// <summary>
    /// エンティティを作成するコマンド
    /// </summary>
    class CreateEntityCommand
        : public IEditCommand {
    public:
        CreateEntityCommand(HierarchyArea* _parentArea, const ::std::string& _entityName);
        ~CreateEntityCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string entityName_;
        OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle(); // 作成するエンティティの名前
    };

    /// <summary>
    /// エンティティをファイルから読み込むコマンド
    /// </summary>
    class LoadEntityCommand
        : public IEditCommand {
    public:
        LoadEntityCommand(HierarchyArea* _parentArea, const ::std::string& _directory, const ::std::string& _entityName);
        ~LoadEntityCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        ::std::string directory_;
        HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
        ::std::string entityName_;
        OriGine::EntityHandle entityHandle_ = OriGine::EntityHandle(); // 作成するエンティティの名前
    };

    /// <summary>
    /// Entityのデータをコピーするコマンド
    /// </summary>
    class CopyEntityCommand
        : public IEditCommand {
    public:
        CopyEntityCommand(EntityHierarchyRegion* _hierarchy);
        ~CopyEntityCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchyRegion* hierarchy_ = nullptr; // 親エリアへのポインタ
    };

    class PasteEntityCommand
        : public IEditCommand {
    public:
        PasteEntityCommand(EntityHierarchyRegion* _hierarchy);
        ~PasteEntityCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchyRegion* hierarchy_ = nullptr; // 親エリアへのポインタ
        ::std::vector<OriGine::EntityHandle> pastedEntityHandles_; // ペーストしたエンティティのIDリスト
    };

private:
    HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
    ::std::list<OriGine::EntityHandle> selectedEntityHandles_; // 選択されているオブジェクトのIDリスト
    ::std::string searchBuff_ = ""; // 検索バッファ
    ::std::list<nlohmann::json> copyBuffer_; // エンティティのコピー用バッファ
public:
    const ::std::list<OriGine::EntityHandle>& GetSelectedEntityHandles() const {
        return selectedEntityHandles_;
    }
    ::std::list<OriGine::EntityHandle>& GetSelectedEntityHandlesRef() {
        return selectedEntityHandles_;
    }

    HierarchyArea* GetParentArea() const {
        return parentArea_;
    }
};

#endif // _DEBUG
