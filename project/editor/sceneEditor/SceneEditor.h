#pragma once

#include "editor/IEditor.h"

#ifdef _DEBUG

/// stl
#include <list>
#include <string>

/// engine
#include "scene/Scene.h"

/// editor
#include "editor/IEditor.h"

/// ECS
// entity
#include "entity/EntityHandle.h"
// system
#include "system/SystemCategory.h"

/// util
#include "globalVariables/SerializedField.h"

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
    void InitializeScene(const ::std::string& _sceneName);
    void InitializeScene();

    void FinalizeMenus();
    void FinalizeAreas();
    void FinalizeScene();

private:
    void LoadNextScene();

private:
    ::std::unique_ptr<OriGine::Scene> currentScene_; // 現在のシーン
    bool isSceneChanged_                                   = false; // シーンが変更されたかどうか
    ::std::string nextSceneName_                           = ""; // 次に読み込むシーン名
    OriGine::SerializedField<::std::string> editSceneName_ = OriGine::SerializedField<::std::string>("Settings", "SceneEditor", "editSceneName", "Game"); // 編集中のシーン名(保存する)
public:
    OriGine::Scene* GetCurrentScene() {
        return currentScene_.get();
    }
    void ChangeScene(const ::std::string& _nextScene) {
        nextSceneName_  = _nextScene;
        isSceneChanged_ = true;
    }
    OriGine::SerializedField<::std::string>& GetEditSceneName() {
        return editSceneName_;
    }
};

/// <summary>
/// コンポーネントを追加するコマンド
/// </summary>
class AddComponentCommand
    : public IEditCommand {
public:
    AddComponentCommand(const ::std::list<OriGine::EntityHandle>& _entityHandles, const ::std::string& _compTypeName)
        : entityHandles_(_entityHandles), componentTypeName_(_compTypeName) {
    }
    ~AddComponentCommand() override = default;

    void Execute() override;
    void Undo() override;

private:
    ::std::list<OriGine::EntityHandle> entityHandles_;
    ::std::string componentTypeName_; // 追加するコンポーネントのタイプ名
};
/// <summary>
/// コンポーネントを削除するコマンド
/// </summary>
class RemoveComponentCommand
    : public IEditCommand {
public:
    RemoveComponentCommand(OriGine::EntityHandle _entityHandle, const ::std::string& _compTypeName, int32_t _compIndex)
        : entityHandle_(_entityHandle), componentTypeName_(_compTypeName), componentIndex_(_compIndex) {}
    ~RemoveComponentCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    OriGine::EntityHandle entityHandle_;

    // nlohmann::json componentData_; // 削除するコンポーネントのデータ
    int32_t componentIndex_ = -1; // 削除するコンポーネントのインデックス
    ::std::string componentTypeName_; // 削除するコンポーネントのタイプ名
};

/// <summary>
/// システムを追加するコマンド
/// </summary>
class AddSystemCommand
    : public IEditCommand {
public:
    AddSystemCommand(const ::std::list<OriGine::EntityHandle>& _entityHandles, const ::std::string& _systemTypeName, OriGine::SystemCategory _category);
    ~AddSystemCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    ::std::list<OriGine::EntityHandle> entityHandles_; // 対象のエンティティIDリスト
    ::std::string systemTypeName_; // 追加するシステムのタイプ名
    OriGine::SystemCategory systemCategory_; // システムのカテゴリ
};
/// <summary>
/// システムを削除するコマンド
/// </summary>
class RemoveSystemCommand
    : public IEditCommand {
public:
    RemoveSystemCommand(const ::std::list<OriGine::EntityHandle>& _entityHandles, const ::std::string& _systemTypeName, OriGine::SystemCategory _category);
    ~RemoveSystemCommand() override = default;
    void Execute() override;
    void Undo() override;

private:
    ::std::list<OriGine::EntityHandle> entityHandles_; // 対象のエンティティIDリスト
    ::std::string systemTypeName_; // 削除するシステムのタイプ名
    OriGine::SystemCategory systemCategory_; // システムのカテゴリ
};

#endif // _DEBUG
