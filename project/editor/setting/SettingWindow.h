#pragma once
#ifdef _DEBUG

/// parent
#include "editor/IEditor.h"

/// util
#include "util/globalVariables/SerializedField.h"

/// math
#include <math/Vector2.h>

namespace OriGine {
namespace Editor {

/// <summary>
/// 設定メニュー
/// </summary>
class SettingsMenu
    : public Menu {
public:
    SettingsMenu();
    ~SettingsMenu() override;
    void Initialize() override;
    void Finalize() override;

private:
};
/// <summary>
/// 設定ウィンドウを開くメニューアイテム
/// </summary>
class SettingsWindowOpen
    : public MenuItem {
public:
    SettingsWindowOpen();
    ~SettingsWindowOpen() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
};

/// <summary>
/// 設定ウィンドウ
/// </summary>
class SettingWindow
    : public Window {
public:
    /// globalVariables で 設定を保存しているシーン名
    static const std::string kGlobalVariablesSceneName;

public:
    SettingWindow();
    ~SettingWindow() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;
};

/// <summary>
/// 設定ウィンドウ内のエリア
/// </summary>
class SettingWindowArea
    : public Area {
public:
    SettingWindowArea();
    ~SettingWindowArea() override;
    void Initialize() override;
    void Finalize() override;
};
class SettingWindowRegion
    : public Region {
public:
    static const std::string kGlobalVariablesGroupName;

public:
    SettingWindowRegion();
    ~SettingWindowRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    std::string windowTitle_ = "OriGine"; // ウィンドウのタイトル
    Vec2f windowSize_        = Vec2f(1280.f, 720.f); // ウィンドウのサイズ
};

/// <summary>
/// プロジェクト設定エリア
/// </summary>
class ProjectSettingArea
    : public Area {
public:
    ProjectSettingArea();
    ~ProjectSettingArea() override;
    void Initialize() override;
    void Finalize() override;
};
/// <summary>
/// プロジェクト設定リージョン
/// </summary>
class ProjectSettingRegion
    : public Region {
public:
    ProjectSettingRegion();
    ~ProjectSettingRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    //! TODO : 設定項目の追加
    std::string startUpSceneName_ = "Game"; // 起動時のシーン名

    // ↓ 本当は 別の場所に保存したいけど項目が少なすぎるため一旦ここ
    float gravity_ = 9.8f; // 重力の値
};

} // namespace Editor
} // namespace OriGine

#endif // _DEBUG
