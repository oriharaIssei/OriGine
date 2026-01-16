#pragma once

#ifdef _DEBUG

/// stl
#include <string>

/// engine
#include "scene/Scene.h"

/// editor
#include "editor/IEditor.h"
#include "editor/sceneEditor/SceneEditor.h"

/// <summary>
/// File関係の操作をまとめたメニュー
/// </summary>
class SceneEditorFileMenu
    : public Editor::Menu {
public:
    SceneEditorFileMenu(SceneEditorWindow* _parentWindow);
    ~SceneEditorFileMenu() override;
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
    SaveMenuItem(SceneEditorFileMenu* _parent);
    ~SaveMenuItem() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    SceneEditorFileMenu* parentMenu_ = nullptr; // 親メニューへのポインタ
};

/// <summary>
/// シーンファイルを読み込むメニューアイテム
/// </summary>
class LoadMenuItem
    : public Editor::MenuItem {
public:
    LoadMenuItem(SceneEditorFileMenu* _parent);
    ~LoadMenuItem();

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    SceneEditorFileMenu* parentMenu_ = nullptr; // 親メニューへのポインタ
    OriGine::Scene* loadScene_       = nullptr; // 保存するシーンへのポインタ
};

/// <summary>
/// シーンファイルを新規作成するメニューアイテム
/// </summary>
class CreateMenuItem
    : public Editor::MenuItem {
public:
    CreateMenuItem(SceneEditorFileMenu* _parent);
    ~CreateMenuItem();

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    SceneEditorFileMenu* parentMenu_ = nullptr; // 親メニューへのポインタ
    ::std::string newSceneName_      = "";
};

#endif // _DEBUG
