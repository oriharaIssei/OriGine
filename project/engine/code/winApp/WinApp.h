#pragma once

/// api
#include <Windows.h>

/// stl
#include <memory>
#include <string>

/// math
#include <cstdint>
#include <Vector2.h>

namespace OriGine {

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
/// Windows アプリケーションの基盤 (ウィンドウ生成、メッセージループ) を管理するクラス.
/// OS レベルのイベントを抽象化し、エンジン各部へのアクティブ状態の通知などを行う.
/// </summary>
class WinApp {
public:
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

private:
    HWND hwnd_                              = nullptr; // ウィンドウハンドル
    ::std::unique_ptr<WNDCLASSEX> wndClass_ = nullptr; // ウィンドウクラス定義
    UINT windowStyle_; // 生成時のウィンドウスタイル

    ::std::wstring wideWindowTitle_; // ウィンドウタイトル兼クラス名
    RECT windowRect_{}; // ウィンドウ矩形領域
    float aspectRatio_ = 0.0f; // 目標アスペクト比
    Vec2f windowSize_; // クライアント領域のサイズ (float)
    int32_t clientWidth_, clientHeight_; // クライアント領域の整数サイズ

    WindowResizeMode windowResizeMode_ = WindowResizeMode::FIXED_ASPECT; // リサイズ方針
    bool isFullscreen_                 = false; // フルスクリーン中フラグ
    bool isReSized_                    = false; // サイズ変更発生フラグ
    bool isActive_                     = false; // ウィンドウがアクティブかどうか

public:
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
