#pragma once

/// api
#include <Windows.h>

/// stl
#include <memory>
#include <string>

/// lib
#include "globalVariables/SerializedField.h"

/// math
#include <cstdint>

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

private:
    HWND hwnd_                            = nullptr; // ウィンドウハンドル
    std::unique_ptr<WNDCLASSEX> wndClass_ = nullptr; // ウィンドウクラス
    UINT windowStyle_;

    std::wstring wideWindowTitle_; // ウィンドウクラス名も兼ねている
    Vec2f windowSize_; // ウィンドウサイズ
    int32_t clientWidth_, clientHeight_;

    bool isReSized_ = false;

public:
    bool isReSized() const { return isReSized_; }
    void setIsReSized(bool _isResized) { isReSized_ = _isResized; }

    const HINSTANCE& getHInstance() const { return wndClass_->hInstance; }
    const HWND& getHwnd() const { return hwnd_; }

    const Vec2f& getWindowSize() const { return windowSize_; }

    const int32_t& getWidth() const { return clientWidth_; }
    const int32_t& getHeight() const { return clientHeight_; }
};
