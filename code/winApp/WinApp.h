#pragma once

/// api
#include <Windows.h>

/// stl
#include <functional>
#include <memory>
#include <string>
#include <vector>

/// math
#include <cstdint>
#include <Vector2.h>

/// engine
#include "EngineConfig.h"

namespace OriGine {

using Vec2i = Vector2<int32_t>;

/// <summary>
/// ウィンドウのサイズ変更挙動を制御する列挙型.
/// ビットフラグとして組み合わせ可能.
/// </summary>
enum class WindowResizeMode {
    NONE         = 0b0, // リサイズ不可
    FREE         = 0b1, // 自由に変更可能
    FIXED        = 0b10, // サイズ固定
    FIXED_WIDTH  = 0b100, // 幅のみ固定
    FIXED_HEIGHT = 0b1000, // 高さのみ固定
    FIXED_ASPECT = 0b10000, // 指定されたアスペクト比を維持してリサイズ
};

/// <summary>
/// ウィンドウの表示形態（通常ウィンドウ／ボーダレス／排他フルスクリーン等）を表す列挙型.
/// </summary>
enum class WindowMode {
    WINDOWED,
    BORDERLESS_WINDOWED,
    BORDERLESS_FULLSCREEN,
    EXCLUSIVE_FULLSCREEN,
};

/// <summary>
/// モニタがサポートする 1 つの表示解像度・リフレッシュレートの組.
/// </summary>
struct DisplayMode {
    int32_t width       = 0;
    int32_t height      = 0;
    int32_t refreshRate = 0;
};

/// <summary>
/// 接続されているモニタ 1 台分の情報（作業領域・DPI・プライマリかどうか等）.
/// </summary>
struct MonitorInfo {
    ::std::wstring name;
    RECT workArea{};
    RECT monitorArea{};
    UINT dpi       = Config::Window::kDefaultDpi;
    bool isPrimary = false;
};

/// <summary>
/// ウィンドウ生成時に指定する各種初期設定をまとめた構造体.
/// </summary>
struct WindowDesc {
    const wchar_t* title     = L"OriGine Application";
    int32_t clientWidth      = Config::Window::kDefaultClientWidth;
    int32_t clientHeight     = Config::Window::kDefaultClientHeight;
    UINT windowStyle         = WS_OVERLAPPEDWINDOW;
    int32_t minWidth         = Config::Window::kDefaultMinWidth;
    int32_t minHeight        = Config::Window::kDefaultMinHeight;
    int32_t maxWidth         = Config::Window::kDefaultMaxWidth;
    int32_t maxHeight        = Config::Window::kDefaultMaxHeight;
    WindowResizeMode resizeMode = WindowResizeMode::FIXED_ASPECT;
    const wchar_t* iconPath  = nullptr;
    int iconResourceId       = 0;
    WindowMode windowMode    = WindowMode::WINDOWED;
    bool showCursor          = true;
    bool clipCursor          = false;
    bool enableDpiAwareness  = true;
    bool enableDragDrop      = false;
    bool enableBackgroundTransparency = false;
    BYTE backgroundAlpha     = 255;
    COLORREF transparencyColorKey = RGB(0, 0, 0);
    bool useTransparencyColorKey = true;
};

/// <summary>
/// Windows アプリケーションの基盤 (ウィンドウ生成、メッセージループ) を管理するクラス.
/// OS レベルのイベントを抽象化し、エンジン各部へのアクティブ状態の通知などを行う.
/// </summary>
class WinApp {
public:
    using DropCallback = ::std::function<void(const ::std::vector<::std::wstring>& _paths)>;

    /// <summary>
    /// OS から届くウィンドウメッセージを処理するコールバック関数.
    /// ImGui の入力処理などもここで行う.
    /// </summary>
    static LRESULT WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);

    WinApp() = default;
    ~WinApp();

    /// <summary>
    /// Windows API を使用してゲームウィンドウを生成する.
    /// </summary>
    /// <param name="_title">ウィンドウのキャプション名</param>
    /// <param name="_windowStyle">WS_OVERLAPPEDWINDOW 等のスタイルフラグ</param>
    /// <param name="_clientWidth">クライアント領域の横幅 (ピクセル)</param>
    /// <param name="_clientHeight">クライアント領域の縦幅 (ピクセル)</param>
    void CreateGameWindow(const WindowDesc& _desc);
    void CreateGameWindow(const wchar_t* _title, UINT _windowStyle,
        int32_t _clientWidth, int32_t _clientHeight);

    /// <summary>
    /// ウィンドウクラスの解除とウィンドウの破棄を行う.
    /// </summary>
    void TerminateGameWindow();

    /// <summary>
    /// PeekMessage を使用してウィンドウメッセージをキューから取り出し、処理する.
    /// </summary>
    /// <returns>WM_QUIT を受信した場合は false, それ以外は true</returns>
    bool ProcessMessage();

    /// <summary>
    /// フォアグラウンドウィンドウかどうかの判定に基づき、内部の isActive_ 状態を更新する.
    /// </summary>
    void UpdateActivity();

    /// <summary>
    /// フルスクリーンモードとウィンドウモードを切り替える.
    /// 切替時にはウィンドウスタイルの書き換えを行う.
    /// </summary>
    /// <param name="_enable">true でフルスクリーン化</param>
    void ToggleFullscreen(bool _enable);

    /// <summary> ウィンドウの最小サイズを設定する. </summary>
    void SetMinWindowSize(int32_t _width, int32_t _height);
    /// <summary> ウィンドウの最大サイズを設定する. </summary>
    void SetMaxWindowSize(int32_t _width, int32_t _height);
    Vec2i GetMinWindowSize() const { return Vec2i(minWidth_, minHeight_); }
    Vec2i GetMaxWindowSize() const { return Vec2i(maxWidth_, maxHeight_); }

    UINT GetDpi() const { return currentDpi_; }
    float GetDpiScale() const { return dpiScale_; }
    bool IsDpiAware() const { return dpiAware_; }

    /// <summary> マウスカーソルの表示・非表示を切り替える. </summary>
    void ShowCursor(bool _show);
    bool IsCursorVisible() const { return cursorVisible_; }
    /// <summary> カーソルの移動範囲をウィンドウ内に制限する（クリッピング）. </summary>
    void SetCursorClip(bool _clip);
    bool IsCursorClipped() const { return cursorClipped_; }
    /// <summary> 指定パスの画像からカスタムカーソルを読み込み、適用する. </summary>
    void SetCustomCursor(const wchar_t* _cursorPath);
    /// <summary> カーソルを OS 標準のものへ戻す. </summary>
    void ResetCursor();

    /// <summary> 指定パスのアイコンファイルをウィンドウアイコンとして設定する. </summary>
    void SetIcon(const wchar_t* _iconPath);
    /// <summary> リソース ID を指定してウィンドウアイコンを設定する. </summary>
    void SetIcon(int _resourceId);

    /// <summary> 指定モニタがサポートする表示解像度・リフレッシュレートの一覧を取得する. </summary>
    static ::std::vector<DisplayMode> EnumerateDisplayModes(int _monitorIndex = 0);
    /// <summary> クライアント領域の解像度を変更する. </summary>
    void ChangeResolution(int32_t _width, int32_t _height);

    /// <summary> ウィンドウ表示モード（通常・ボーダレス・排他フルスクリーン等）を切り替える. </summary>
    void SetWindowMode(WindowMode _mode);
    WindowMode GetWindowMode() const { return windowMode_; }
    bool IsFullscreen() const {
        return windowMode_ == WindowMode::BORDERLESS_FULLSCREEN
            || windowMode_ == WindowMode::EXCLUSIVE_FULLSCREEN;
    }

    /// <summary> 現在のウィンドウ位置・サイズ・モード等の状態を保存する. </summary>
    void SaveWindowState();
    /// <summary> 保存済みのウィンドウ状態を復元する. </summary>
    /// <returns>復元できた場合 true</returns>
    bool RestoreWindowState();

    /// <summary> ファイル・フォルダのドラッグ&ドロップ受信時に呼び出すコールバックを登録する. </summary>
    void SetDropCallback(const DropCallback& _callback);
    /// <summary> 登録済みのドロップコールバックを解除する. </summary>
    void ClearDropCallback();

    /// <summary> ウィンドウタイトルを変更する. </summary>
    void SetWindowTitle(const wchar_t* _title);
    void SetWindowTitle(const ::std::wstring& _title) { SetWindowTitle(_title.c_str()); }
    const ::std::wstring& GetWindowTitle() const { return wideWindowTitle_; }

    /// <summary> 接続されているモニタの一覧情報を取得する. </summary>
    static ::std::vector<MonitorInfo> EnumerateMonitors();
    /// <summary> ウィンドウを配置する対象モニタを指定する. </summary>
    void SetTargetMonitor(int _index);
    /// <summary> 現在ウィンドウが表示されているモニタのインデックスを取得する. </summary>
    int GetCurrentMonitorIndex() const;

    /// <summary> ウィンドウを常に最前面に表示するかどうかを設定する. </summary>
    void SetAlwaysOnTop(bool _enable);
    bool IsAlwaysOnTop() const { return alwaysOnTop_; }

    /// <summary> ウィンドウ背景の半透明化（レイヤードウィンドウ）を有効・無効にする. </summary>
    void SetBackgroundTransparency(bool _enable);
    bool IsBackgroundTransparent() const { return backgroundTransparent_; }
    /// <summary> ウィンドウ全体の不透明度を設定する. </summary>
    void SetWindowOpacity(BYTE _alpha);
    BYTE GetWindowOpacity() const { return backgroundAlpha_; }
    /// <summary> 指定した色を透過色として扱うカラーキー透過を設定する. </summary>
    void SetTransparencyColorKey(COLORREF _colorKey, bool _enable);
    COLORREF GetTransparencyColorKey() const { return transparencyColorKey_; }
    bool IsTransparencyColorKeyEnabled() const { return useTransparencyColorKey_; }

    // ===== グローバルホットキー =====
    /// <summary> OS 全体で有効なグローバルホットキーを登録する. </summary>
    /// <param name="_id">ホットキーを識別する ID</param>
    /// <param name="_modifiers">Ctrl/Alt 等の修飾キーフラグ</param>
    /// <param name="_vk">仮想キーコード</param>
    /// <returns>登録に成功したか</returns>
    bool RegisterGlobalHotkey(int _id, UINT _modifiers, UINT _vk);
    /// <summary> 指定 ID のグローバルホットキーを解除する. </summary>
    void UnregisterGlobalHotkey(int _id);
    /// <summary> 登録済みの全グローバルホットキーを解除する. </summary>
    void UnregisterAllHotkeys();

    // ===== システムトレイ =====
    using TrayMenuCallback = ::std::function<void()>;

    /// <summary> システムトレイにアイコンを表示する. </summary>
    bool EnableSystemTray(const wchar_t* _tooltip, HICON _icon = nullptr);
    /// <summary> システムトレイアイコンを非表示にする. </summary>
    void DisableSystemTray();
    /// <summary> トレイアイコンのツールチップ文字列を変更する. </summary>
    void SetTrayTooltip(const wchar_t* _tooltip);
    /// <summary> ウィンドウを非表示にし、システムトレイへ最小化する. </summary>
    void MinimizeToTray();
    /// <summary> トレイに格納されたウィンドウを元の状態へ復元する. </summary>
    void RestoreFromTray();
    bool IsMinimizedToTray() const { return minimizedToTray_; }
    void SetMinimizeToTrayOnClose(bool _enable) { minimizeToTrayOnClose_ = _enable; }
    bool IsMinimizeToTrayOnClose() const { return minimizeToTrayOnClose_; }

    /// <summary>
    /// トレイアイコン右クリック時のコンテキストメニューに表示する項目.
    /// </summary>
    struct TrayMenuItem {
        ::std::wstring label;
        TrayMenuCallback callback;
    };
    /// <summary> トレイのコンテキストメニューに項目を追加する. </summary>
    void AddTrayMenuItem(const wchar_t* _label, TrayMenuCallback _callback);
    /// <summary> トレイのコンテキストメニュー項目をすべて削除する. </summary>
    void ClearTrayMenu();

    // ===== 自動起動 =====
    /// <summary> OS ログイン時の自動起動設定を行う（レジストリ登録）. </summary>
    static bool SetAutoStart(const wchar_t* _appName, bool _enable);
    /// <summary> 自動起動が有効になっているかどうかを取得する. </summary>
    static bool IsAutoStartEnabled(const wchar_t* _appName);

private:
    /// <summary> cursorClipped_ の状態に応じてカーソルの移動範囲をウィンドウ内へ実際にクリップする. </summary>
    void ApplyCursorClip();
    /// <summary> 背景透過・不透明度・カラーキーの設定をレイヤードウィンドウ属性へ反映する. </summary>
    void ApplyBackgroundTransparency();
    /// <summary> 自前で生成・所有しているアイコンハンドルを解放する. </summary>
    void ReleaseOwnedIcons();
    /// <summary> 指定インデックスのモニタの矩形範囲を取得する. </summary>
    RECT GetMonitorRect(int _monitorIndex) const;

    HWND hwnd_                              = nullptr; // ウィンドウハンドル
    ::std::unique_ptr<WNDCLASSEX> wndClass_ = nullptr; // ウィンドウクラス定義
    UINT windowStyle_; // 生成時のウィンドウスタイル

    ::std::wstring windowClassName_; // 登録時のウィンドウクラス名
    ::std::wstring wideWindowTitle_; // ウィンドウタイトル兼クラス名
    RECT windowRect_{}; // ウィンドウ矩形領域
    float aspectRatio_ = 0.0f; // 目標アスペクト比
    Vec2f windowSize_; // クライアント領域のサイズ (float)
    int32_t clientWidth_, clientHeight_; // クライアント領域の整数サイズ

    WindowResizeMode windowResizeMode_ = WindowResizeMode::FIXED_ASPECT; // リサイズ方針
    bool isReSized_                    = false; // サイズ変更発生フラグ
    bool isActive_                     = false; // ウィンドウがアクティブかどうか

    int32_t minWidth_  = Config::Window::kDefaultMinWidth;
    int32_t minHeight_ = Config::Window::kDefaultMinHeight;
    int32_t maxWidth_  = Config::Window::kDefaultMaxWidth;
    int32_t maxHeight_ = Config::Window::kDefaultMaxHeight;

    UINT currentDpi_ = Config::Window::kDefaultDpi;
    float dpiScale_  = 1.0f;
    bool dpiAware_   = false;

    bool cursorVisible_   = true;
    bool cursorClipped_   = false;
    HCURSOR customCursor_ = nullptr;
    HCURSOR defaultCursor_ = nullptr;

    HICON iconLarge_ = nullptr;
    HICON iconSmall_ = nullptr;
    bool ownsIcons_  = false;

    WindowMode windowMode_ = WindowMode::WINDOWED;
    int targetMonitorIndex_ = 0;
    DropCallback dropCallback_;
    bool alwaysOnTop_ = false;
    bool dragDropEnabled_ = false;
    bool backgroundTransparent_ = false;
    BYTE backgroundAlpha_ = 255;
    COLORREF transparencyColorKey_ = RGB(0, 0, 0);
    bool useTransparencyColorKey_ = true;
    bool clickThrough_ = false;
    bool showTitleBar_ = true;
    bool allowFullscreenToggle_ = true;

    // ホットキー
    ::std::vector<int> registeredHotkeyIds_;

    // システムトレイ
    bool trayEnabled_ = false;
    bool minimizedToTray_ = false;
    bool minimizeToTrayOnClose_ = false;
    ::std::vector<TrayMenuItem> trayMenuItems_;
    /// <summary> トレイアイコン右クリック時に登録済みメニュー項目のポップアップメニューを表示する. </summary>
    void ShowTrayContextMenu();

    // テキスト入力 (WM_CHAR で蓄積。IME 確定文字もここに届く)。自前 UI のテキスト入力欄が消費する。
    ::std::wstring typedCharBuffer_;

public:
    /// <summary>
    /// このフレームに WM_CHAR で届いた入力文字 (UTF-16) を取り出してバッファをクリアする.
    /// バックスペースは 0x08、Enter は 0x0D として届く. 自前 UI のテキスト入力欄から毎フレーム呼ぶ.
    /// </summary>
    ::std::wstring TakeTypedChars() {
        ::std::wstring s;
        s.swap(typedCharBuffer_);
        return s;
    }

    /// <summary>
    /// ウィンドウが現在アクティブ (フォーカスされている) かどうかを取得する.
    /// </summary>
    bool IsActive() const { return isActive_; }

    /// <summary>
    /// 現在のウィンドウリサイズ設定を取得する.
    /// </summary>
    WindowResizeMode GetWindowResizeMode() const { return windowResizeMode_; }

    /// <summary>
    /// ウィンドウのリサイズ挙動ポリシーを設定する.
    /// </summary>
    /// <param name="_windowResizeMode">設定するモード</param>
    void SetWindowResizeMode(WindowResizeMode _windowResizeMode) {
        windowResizeMode_ = _windowResizeMode;
    }

    /// <summary>
    /// ウィンドウがリサイズされたかどうか (フラグ) を取得する.
    /// </summary>
    bool isReSized() const { return isReSized_; }

    /// <summary>
    /// リサイズフラグを更新する. 処理済みの場合に false をセットするために用いる.
    /// </summary>
    /// <param name="_isResized">リサイズ状態</param>
    void SetIsReSized(bool _isResized) { isReSized_ = _isResized; }

    /// <summary>
    /// アプリケーションインスタンスの HINSTANCE を取得する.
    /// </summary>
    const HINSTANCE& GetHInstance() const { return wndClass_->hInstance; }

    /// <summary>
    /// ウィンドウハンドル (HWND) を取得する. DirectX の初期化等に必要.
    /// </summary>
    const HWND& GetHwnd() const { return hwnd_; }

    /// <summary>
    /// 現在のクライアント領域のサイズを取得する.
    /// </summary>
    const Vec2f& GetWindowSize() const { return windowSize_; }

    /// <summary>
    /// クライアント領域の横幅を取得する.
    /// </summary>
    int32_t GetWidth() const { return clientWidth_; }

    /// <summary>
    /// クライアント領域の縦幅を取得する.
    /// </summary>
    int32_t GetHeight() const { return clientHeight_; }
};

/// <summary>
/// コマンドラインでプロセスを実行し、完了を待つ
/// </summary>
bool RunProcessAndWait(const ::std::string& _command, const char* _currentDirectory = nullptr);

} // namespace OriGine
