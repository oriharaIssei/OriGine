#pragma once

#include "editor/IEditor.h"

#ifdef _DEBUG

/// stl
#include <array>
#include <list>
#include <unordered_map>
#include <vector>

#include <atomic>
#include <memory>

#include <string>

/// engine
#include "component/IComponent.h"
#include "scene/Scene.h"
#include "system/ISystem.h"
enum class SystemCategory;
// camera
class DebugCamera;

#include "globalVariables/SerializedField.h"

// util
#include "util/EnumBitmask.h"
#include "util/nameof.h"

/// <summary>
/// 1つのシーンを編集するためのウィンドウ(Editor)
/// </summary>
class SceneEditorWindow
    : public Editor::Window {
public:
    SceneEditorWindow() : Editor::Window(nameof<SceneEditorWindow>()) {}
    ~SceneEditorWindow() {}
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

    void InitializeMenus();
    void InitializeAreas();
    void InitializeScene(const std::string& _sceneName);
    void InitializeScene();

    void FinalizeMenus();
    void FinalizeAreas();
    void FinalizeScene();

private:
    void LoadNextScene();

private:
    std::unique_ptr<Scene> currentScene_; // 現在のシーン
    bool isSceneChanged_                        = false; // シーンが変更されたかどうか
    std::string nextSceneName_                  = ""; // 次に読み込むシーン名
    SerializedField<std::string> editSceneName_ = SerializedField<std::string>("Settings", "SceneEditor", "editSceneName", "Game"); // 編集中のシーン名(保存する)
public:
    Scene* GetCurrentScene() {
        return currentScene_.get();
    }
    void ChangeScene(const std::string& _nextScene) {
        nextSceneName_  = _nextScene;
        isSceneChanged_ = true;
    }
    SerializedField<std::string>& GetEditSceneName() {
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
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
};
/// <summary>
/// シーンファイルを保存するメニューアイテム
/// </summary>
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
};
/// <summary>
/// シーンファイルを読み込むメニューアイテム
/// </summary>
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
/// <summary>
/// シーンファイルを新規作成するメニューアイテム
/// </summary>
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

/// <summary>
/// シーンビューエリア(シーンのDebug描画を確認する)
/// </summary>
class SceneViewArea
    : public Editor::Area {
public:
    SceneViewArea(SceneEditorWindow* _parentWindow);
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    void DrawScene();
    void UseImGuizmo(const ImVec2& _sceneViewPos, const Vec2f& _originalResolution);

private:
    SceneEditorWindow* parentWindow_; // 親ウィンドウへのポインタ

    std::unique_ptr<DebugCamera> debugCamera_; // デバッグカメラ
};

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
class EntityHierarchy
    : public Editor::Region {
public:
    EntityHierarchy(HierarchyArea* _parent);
    ~EntityHierarchy() override;
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
        AddSelectedEntitiesCommand(EntityHierarchy* _hierarchy, int32_t _addedEntityId);
        ~AddSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        int32_t addedEntityId_      = -1; // 追加されたエンティティID
    };
    /// <summary>
    /// 編集するEntityを削除するコマンド
    /// </summary>
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
    /// <summary>
    /// 編集するEntityの選択をクリアするコマンド
    /// </summary>
    class ClearSelectedEntitiesCommand
        : public IEditCommand {
    public:
        ClearSelectedEntitiesCommand(EntityHierarchy* _hierarchy);
        ~ClearSelectedEntitiesCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        std::list<int32_t> previousselectedEntityIds_; // 以前の選択されたエンティティIDのリスト
    };

    /// <summary>
    /// エンティティを作成するコマンド
    /// </summary>
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
    };

    /// <summary>
    /// エンティティをファイルから読み込むコマンド
    /// </summary>
    class LoadEntityCommand
        : public IEditCommand {
    public:
        LoadEntityCommand(HierarchyArea* _parentArea, const std::string& _directory, const std::string& _entityName);
        ~LoadEntityCommand() override = default;

        void Execute() override;
        void Undo() override;

    private:
        std::string directory_;
        HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
        std::string entityName_;
        int32_t entityId_ = -1; // 作成するエンティティの名前
    };

    /// <summary>
    /// Entityのデータをコピーするコマンド
    /// </summary>
    class CopyEntityCommand
        : public IEditCommand {
    public:
        CopyEntityCommand(EntityHierarchy* _hierarchy);
        ~CopyEntityCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
    };

    class PasteEntityCommand
        : public IEditCommand {
    public:
        PasteEntityCommand(EntityHierarchy* _hierarchy);
        ~PasteEntityCommand() override = default;
        void Execute() override;
        void Undo() override;

    private:
        EntityHierarchy* hierarchy_ = nullptr; // 親エリアへのポインタ
        std::vector<int32_t> pastedEntityIds_; // ペーストしたエンティティのIDリスト
    };

private:
    HierarchyArea* parentArea_ = nullptr; // 親エリアへのポインタ
    std::list<int32_t> selectedEntityIds_; // 選択されているオブジェクトのIDリスト
    std::string searchBuff_ = ""; // 検索バッファ
    std::list<nlohmann::json> copyBuffer_; // エンティティのコピー用バッファ
public:
    const std::list<int32_t>& GetSelectedEntityIds() const {
        return selectedEntityIds_;
    }
    std::list<int32_t>& GetSelectedEntityIdsRef() {
        return selectedEntityIds_;
    }

    HierarchyArea* GetParentArea() const {
        return parentArea_;
    }
};

/// <summary>
/// コンポーネントを追加するコマンド
/// </summary>
class AddComponentCommand
    : public IEditCommand {
public:
    AddComponentCommand(const std::list<int32_t>& _entityIds, const std::string& _compTypeName)
        : entityIds_(_entityIds), componentTypeName_(_compTypeName) {
    }
    ~AddComponentCommand() override = default;

    void Execute() override;
    void Undo() override;

private:
    std::list<int32_t> entityIds_;
    std::string componentTypeName_; // 追加するコンポーネントのタイプ名
};
/// <summary>
/// コンポーネントを削除するコマンド
/// </summary>
class RemoveComponentCommand
    : public IEditCommand {
public:
    RemoveComponentCommand(int32_t _entityId, const std::string& _compTypeName, int32_t _compIndex)
        : entityId_(_entityId), componentTypeName_(_compTypeName), componentIndex_(_compIndex) {}
    ~RemoveComponentCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    int32_t entityId_;

    // nlohmann::json componentData_; // 削除するコンポーネントのデータ
    int32_t componentIndex_ = -1; // 削除するコンポーネントのインデックス
    std::string componentTypeName_; // 削除するコンポーネントのタイプ名
};

/// <summary>
/// システムを追加するコマンド
/// </summary>
class AddSystemCommand
    : public IEditCommand {
public:
    AddSystemCommand(const std::list<int32_t>& _entityIds, const std::string& _systemTypeName, SystemCategory _category);
    ~AddSystemCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    std::list<int32_t> entityIds_; // 対象のエンティティIDリスト
    std::string systemTypeName_; // 追加するシステムのタイプ名
    SystemCategory systemCategory_; // システムのカテゴリ
};
/// <summary>
/// システムを削除するコマンド
/// </summary>
class RemoveSystemCommand
    : public IEditCommand {
public:
    RemoveSystemCommand(const std::list<int32_t>& _entityIds, const std::string& _systemTypeName, SystemCategory _category);
    ~RemoveSystemCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    std::list<int32_t> entityIds_; // 対象のエンティティIDリスト
    std::string systemTypeName_; // 削除するシステムのタイプ名
    SystemCategory systemCategory_; // システムのカテゴリ
};

/// <summary>
/// 開発用コントロールエリア
/// </summary>
/// <remarks>
/// Developをビルド・実行するためのエリア
/// </remarks>
class DevelopControlArea
    : public Editor::Area {
public:
public:
    DevelopControlArea(SceneEditorWindow* _parentWindow);
    ~DevelopControlArea() override;

    void Initialize() override;
    // void DrawGui() override;
    // void Finalize() override;

    class ControlRegion
        : public Editor::Region {
    public:
        ControlRegion(DevelopControlArea* _parentArea);
        ~ControlRegion() override;
        void Initialize() override;
        void DrawGui() override;
        void Finalize() override;

    private:
        DevelopControlArea* parentArea_ = nullptr; // 親エリアへのポインタ
    };

private:
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    std::atomic<bool> isBuilding_ = false;

    const std::string buildTool_        = "msbuild"; // ビルドツールの名前
    const std::string projectDirectory_ = "project";
    const std::string projectName_      = "OriGine.sln";
    const std::string configuration     = "Develop";
    const std::string platform          = "x64";

    const std::string exePath_ = "/../generated/output/" + configuration + "/OriGineApp.exe"; // 実行ファイルのパス

public:
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
};

#endif // _DEBUG
