#pragma once

/// api
#include <Windows.h>

/// stl
#include <memory>
#include <string>

#include "globalVariables/SerializedField.h"

/// math
#include <cstdint>

namespace OriGine {

enum class WindowResizeMode {
    NONE         = 0b0, // リサイズしない
    FREE         = 0b1, // 自由にリサイズ
    FIXED        = 0b10, // 固定サイズ
    FIXED_WIDTH  = 0b100, // 幅を固定
    FIXED_HEIGHT = 0b1000, // 高さを固定
    FIXED_ASPECT = 0b10000, // アスペクト比を固定
};

/// <summary>
/// ウィンドウズアプリケーション
/// </summary>
class WinApp {
public:
    static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    WinApp() = default;
    ~WinApp();

    void CreateGameWindow(const wchar_t* title, UINT windowStyle,
        int32_t clientWidth, int32_t clientHeight);

    void TerminateGameWindow();

    bool ProcessMessage();

    void UpdateActivity();

    void ToggleFullscreen(bool enable);

private:
    HWND hwnd_                            = nullptr; // ウィンドウハンドル
    std::unique_ptr<WNDCLASSEX> wndClass_ = nullptr; // ウィンドウクラス
    UINT windowStyle_;

    std::wstring wideWindowTitle_; // ウィンドウクラス名も兼ねている
    RECT windowRect_{};
    float aspectRatio_ = 0.0f; // アスペクト比
    Vec2f windowSize_; // ウィンドウサイズ
    int32_t clientWidth_, clientHeight_;

    WindowResizeMode windowResizeMode_ = WindowResizeMode::FIXED_ASPECT;
    bool isFullscreen_                 = false; // フルスクリーンモードかどうか
    bool isReSized_                    = false;
    bool isActive_                     = false; // アクティブ状態

public:
    bool IsActive() const { return isActive_; }
    WindowResizeMode GetWindowResizeMode() const { return windowResizeMode_; }
    void SetWindowResizeMode(WindowResizeMode _windowResizeMode) {
        windowResizeMode_ = _windowResizeMode;
    }

    bool isReSized() const { return isReSized_; }
    void SetIsReSized(bool _isResized) { isReSized_ = _isResized; }

    const HINSTANCE& GetHInstance() const { return wndClass_->hInstance; }
    const HWND& GetHwnd() const { return hwnd_; }

    const Vec2f& GetWindowSize() const { return windowSize_; }

    const int32_t& GetWidth() const { return clientWidth_; }
    const int32_t& GetHeight() const { return clientHeight_; }
};

/// <summary>
/// コマンドラインでプロセスを実行し、完了を待つ
/// </summary>
bool RunProcessAndWait(const std::string& command, const char* _currentDirectory = nullptr);

}
