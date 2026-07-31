#pragma once

#ifdef _DEBUG

/// interface
#include "editor/IEditor.h"

/// stl
#include <cstdint>

/// <summary>
/// プロファイラウィンドウ.
/// 階層プロファイラ(システム別ms・呼び出し回数・自己/合計時間)、
/// フレームタイムグラフ(平均/99パーセンタイル/最大)、
/// アロケーション状況(フレームあたりの確保/解放回数・バイト数)を表示する.
/// </summary>
class ProfilerWindow
    : public Editor::Window {
public:
    ProfilerWindow();
    ~ProfilerWindow() override;

    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// フレームタイム / アロケーション状況のグラフを表示するエリア
/// </summary>
class ProfilerOverviewArea
    : public Editor::Area {
public:
    ProfilerOverviewArea();
    ~ProfilerOverviewArea() override;
    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// フレームタイム / アロケーション状況を描画するリージョン
/// </summary>
class ProfilerOverviewRegion
    : public Editor::Region {
public:
    ProfilerOverviewRegion();
    ~ProfilerOverviewRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;
};

/// <summary>
/// システムごとの階層プロファイル結果を表示するエリア
/// </summary>
class ProfilerTreeArea
    : public Editor::Area {
public:
    ProfilerTreeArea();
    ~ProfilerTreeArea() override;
    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// 階層プロファイル結果(ツリー・ソート可能テーブル)を描画するリージョン
/// </summary>
class ProfilerTreeRegion
    : public Editor::Region {
public:
    ProfilerTreeRegion();
    ~ProfilerTreeRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    int32_t sortMode_    = 0; // 0:合計時間 1:自己時間 2:呼び出し回数 3:名前
    bool sortDescending_ = true;
};

/// <summary>
/// メインメニューに"Profiler"項目を追加するメニュー
/// </summary>
class ProfilerMenu
    : public Editor::Menu {
public:
    ProfilerMenu();
    ~ProfilerMenu() override;
    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// プロファイラウィンドウを開くメニューアイテム
/// </summary>
class ProfilerWindowOpenItem
    : public Editor::MenuItem {
public:
    ProfilerWindowOpenItem();
    ~ProfilerWindowOpenItem() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;
};

#endif // _DEBUG
