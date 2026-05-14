#include "winApp/WinApp.h"

/// stl
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <set>
#include <sstream>
#include <tuple>

#include <vector>

/// api
#include <dwmapi.h>
#include <shellapi.h>

#pragma comment(lib, "dwmapi.lib")

/// engine
#include "EngineConfig.h"

/// util
#include "globalVariables/SerializedField.h"
/// external
#include "logger/Logger.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// ウィンドウメッセージプロシージャの実装.
/// OS から通知される各種イベント (リサイズ、終了要求、システムコマンド等) をハンドリングする.
/// </summary>
LRESULT WinApp::WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam) {
#ifdef _DEBUG
    // ImGui の入力を優先的に処理
    if (ImGui_ImplWin32_WndProcHandler(_hwnd, _msg, _wparam, _lparam)) {
        return true;
    }
#endif // _DEBUG

    // インスタンスポインタを取得
    WinApp* pThis = reinterpret_cast<WinApp*>(GetWindowLongPtr(_hwnd, GWLP_USERDATA));

    // ウィンドウイベントに対する処理の分岐
    switch (_msg) {
    case WM_DESTROY: // ウィンドウの破棄
        PostQuitMessage(0); // OS にアプリケーション終了を通知
        return 0;

    case WM_GETMINMAXINFO: {
        if (pThis) {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(_lparam);
            if (pThis->minWidth_ > 0 && pThis->minHeight_ > 0) {
                mmi->ptMinTrackSize.x = pThis->minWidth_;
                mmi->ptMinTrackSize.y = pThis->minHeight_;
            }
            if (pThis->maxWidth_ > 0 && pThis->maxHeight_ > 0) {
                mmi->ptMaxTrackSize.x = pThis->maxWidth_;
                mmi->ptMaxTrackSize.y = pThis->maxHeight_;
            }
        }
        return 0;
    }

    case WM_DPICHANGED: {
        if (pThis) {
            UINT dpi           = HIWORD(_wparam);
            pThis->currentDpi_ = dpi;
            pThis->dpiScale_   = static_cast<float>(dpi) / static_cast<float>(Config::Window::kDefaultDpi);

            const RECT* suggested = reinterpret_cast<const RECT*>(_lparam);
            SetWindowPos(_hwnd, nullptr,
                suggested->left, suggested->top,
                suggested->right - suggested->left,
                suggested->bottom - suggested->top,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    }

    case WM_SETFOCUS:
        if (pThis && pThis->cursorClipped_) {
            pThis->ApplyCursorClip();
        }
        break;

    case WM_KILLFOCUS:
        ClipCursor(nullptr);
        break;

    case WM_SETCURSOR:
        if (LOWORD(_lparam) == HTCLIENT && pThis) {
            SetCursor(pThis->cursorVisible_ ? (pThis->customCursor_ ? pThis->customCursor_ : pThis->defaultCursor_) : nullptr);
            return TRUE;
        }
        break;

    case WM_DROPFILES: {
        if (pThis && pThis->dropCallback_) {
            HDROP hDrop = reinterpret_cast<HDROP>(_wparam);
            UINT count  = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

            std::vector<std::wstring> paths;
            paths.reserve(count);
            for (UINT i = 0; i < count; ++i) {
                UINT len = DragQueryFileW(hDrop, i, nullptr, 0) + 1;
                std::wstring path(len, L'\0');
                DragQueryFileW(hDrop, i, path.data(), len);
                path.pop_back();
                paths.push_back(std::move(path));
            }
            DragFinish(hDrop);
            pThis->dropCallback_(paths);
        }
        return 0;
    }

    case WM_SYSCOMMAND:
        // 最大化ボタン押下時にフルスクリーンモードに移行するカスタム挙動
        if ((_wparam & 0xFFF0) == SC_MAXIMIZE) {
            if (pThis) {
                pThis->ToggleFullscreen(true);
            }
            return 0;
        } else if ((_wparam & 0xFFF0) == SC_RESTORE) {
            // 元に戻すボタン押下時にフルスクリーンを解除
            if (pThis) {
                pThis->ToggleFullscreen(false);
            }
            return 0;
        }
        break;

    case WM_SIZING: {
        // FIXED_ASPECT モード時、ドラッグ方向に応じてアスペクト比を維持するように矩形を補正
        if (pThis && pThis->windowResizeMode_ == WindowResizeMode::FIXED_ASPECT) {
            RECT* rect   = reinterpret_cast<RECT*>(_lparam);
            int width    = rect->right - rect->left;
            int height   = rect->bottom - rect->top;
            float aspect = pThis->aspectRatio_;

            switch (_wparam) {
            case WMSZ_RIGHT: {
                // 右端ドラッグ時
                width        = rect->right - rect->left;
                height       = int(width / aspect);
                rect->bottom = rect->top + height;
                break;
            }
            case WMSZ_LEFT: {
                // 左端ドラッグ時
                width     = rect->right - rect->left;
                height    = int(width / aspect);
                rect->top = rect->bottom - height;
                break;
            }
            case WMSZ_TOP: {
                // 上端ドラッグ時
                height      = rect->bottom - rect->top;
                width       = int(height * aspect);
                rect->right = rect->left + width;
                break;
            }
            case WMSZ_BOTTOM: {
                // 下端ドラッグ時
                height      = rect->bottom - rect->top;
                width       = int(height * aspect);
                rect->right = rect->left + width;
                break;
            }
            case WMSZ_TOPRIGHT: {
                // 右上ドラッグ時
                width     = rect->right - rect->left;
                height    = int(width / aspect);
                rect->top = rect->bottom - height;
                break;
            }
            case WMSZ_TOPLEFT: {
                // 左上ドラッグ時
                width     = rect->right - rect->left;
                height    = int(width / aspect);
                rect->top = rect->bottom - height;
                break;
            }
            case WMSZ_BOTTOMRIGHT: {
                // 右下を動かす
                width        = rect->right - rect->left;
                height       = int(width / aspect);
                rect->bottom = rect->top + height;
                break;
            }
            case WMSZ_BOTTOMLEFT: {
                // 左下を動かす
                width        = rect->right - rect->left;
                height       = int(width / aspect);
                rect->bottom = rect->top + height;
                break;
            }
            }
            return TRUE;
        }
        break;
    }
    case WM_SIZE: {
        if (pThis == nullptr) {
            return DefWindowProc(_hwnd, _msg, _wparam, _lparam); // デフォルトの処理
        }
        if (_wparam != SIZE_MINIMIZED) {
            pThis->isReSized_ = true;

            pThis->clientWidth_  = LOWORD(_lparam);
            pThis->clientHeight_ = HIWORD(_lparam);
            pThis->windowSize_   = Vec2f(float(pThis->clientWidth_), float(pThis->clientHeight_));
            pThis->ApplyCursorClip();
        }

        break;
    }
    case WM_MOVE:
        if (pThis) {
            pThis->ApplyCursorClip();
        }
        break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
        if (_wparam == VK_F11 && pThis) {
            pThis->ToggleFullscreen(!pThis->IsFullscreen());
            return 0;
        }
    case WM_KEYUP:
    case WM_IME_KEYDOWN:
    case WM_IME_KEYUP:
        if ((_wparam == VK_MENU) || _wparam == VK_F10) {
            return 0;
        }
        break;
    default:
        break;
    }
    return DefWindowProc(_hwnd, _msg, _wparam, _lparam); // デフォルトの処理
}

WinApp::~WinApp() {
    ReleaseOwnedIcons();
    ResetCursor();
    CloseWindow(hwnd_);
}

void WinApp::ReleaseOwnedIcons() {
    if (!ownsIcons_) {
        iconLarge_ = nullptr;
        iconSmall_ = nullptr;
        return;
    }
    if (iconLarge_) {
        DestroyIcon(iconLarge_);
        iconLarge_ = nullptr;
    }
    if (iconSmall_) {
        DestroyIcon(iconSmall_);
        iconSmall_ = nullptr;
    }
    ownsIcons_ = false;
}

void WinApp::ApplyCursorClip() {
    if (!cursorClipped_ || hwnd_ == nullptr || GetForegroundWindow() != hwnd_) {
        return;
    }

    RECT rect{};
    GetClientRect(hwnd_, &rect);
    MapWindowPoints(hwnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2);
    ClipCursor(&rect);
}

void WinApp::ApplyBackgroundTransparency() {
    if (hwnd_ == nullptr) {
        return;
    }

    LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
    if (!backgroundTransparent_) {
        SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
        MARGINS margins{0, 0, 0, 0};
        DwmExtendFrameIntoClientArea(hwnd_, &margins);
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        return;
    }

    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

    DWORD flags = LWA_ALPHA;
    if (useTransparencyColorKey_) {
        flags |= LWA_COLORKEY;
    }
    SetLayeredWindowAttributes(hwnd_, transparencyColorKey_, backgroundAlpha_, flags);

    MARGINS margins{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

RECT WinApp::GetMonitorRect(int _monitorIndex) const {
    auto monitors = EnumerateMonitors();
    if (monitors.empty()) {
        return RECT{0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    }

    int index = std::clamp(_monitorIndex, 0, static_cast<int>(monitors.size()) - 1);
    return monitors[index].monitorArea;
}

/// <summary>
/// フルスクリーンとウィンドウモードの動的切り替え.
/// フルスクリーン時は WS_POPUP を使用して画面全体に広げ、ウィンドウモード時は WS_OVERLAPPEDWINDOW に戻す.
/// </summary>
void WinApp::ToggleFullscreen(bool _enable) {
    SetWindowMode(_enable ? WindowMode::BORDERLESS_FULLSCREEN : WindowMode::WINDOWED);
};

/// <summary>
/// ゲーム用ウィンドウを生成し、表示を開始する.
/// COM の初期化、ウィンドウクラスの登録、CreateWindowAPI の呼び出しを実施する.
/// </summary>
void WinApp::CreateGameWindow(const wchar_t* _title, UINT _windowStyle, int32_t _clientWidth, int32_t _clientHeight) {
    WindowDesc desc{};
    desc.title        = _title;
    desc.windowStyle  = _windowStyle;
    desc.clientWidth  = _clientWidth;
    desc.clientHeight = _clientHeight;
    CreateGameWindow(desc);
}

void WinApp::CreateGameWindow(const WindowDesc& _desc) {
    if (_desc.enableDpiAwareness) {
        dpiAware_ = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE;
    }

    // COM (Component Object Model) の初期化. DirectX や WIC 等で必要.
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    wideWindowTitle_ = _desc.title ? _desc.title : L"OriGine Application";
    windowClassName_ = wideWindowTitle_;
    windowStyle_     = _desc.windowStyle;
    windowResizeMode_ = _desc.resizeMode;
    windowMode_      = _desc.windowMode;
    backgroundTransparent_ = _desc.enableBackgroundTransparency;
    backgroundAlpha_ = _desc.backgroundAlpha;
    transparencyColorKey_ = _desc.transparencyColorKey;
    useTransparencyColorKey_ = _desc.useTransparencyColorKey;

    clientWidth_  = _desc.clientWidth;
    clientHeight_ = _desc.clientHeight;
    windowSize_   = Vec2f(float(_desc.clientWidth), float(_desc.clientHeight));
    aspectRatio_  = windowSize_[X] / windowSize_[Y];
    SetMinWindowSize(_desc.minWidth, _desc.minHeight);
    SetMaxWindowSize(_desc.maxWidth, _desc.maxHeight);

    // ウィンドウクラス (WNDCLASSEX) の構造体設定
    wndClass_                = std::make_unique<WNDCLASSEX>();
    wndClass_->cbSize        = sizeof(WNDCLASSEX);
    wndClass_->lpfnWndProc   = (WNDPROC)WindowProc; // コールバック関数の指定
    wndClass_->lpszClassName = windowClassName_.c_str(); // 識別用のクラス名
    wndClass_->hInstance     = GetModuleHandle(nullptr); // 実行インスタンス
    wndClass_->hCursor       = LoadCursor(NULL, IDC_ARROW); // デフォルトカーソル
    defaultCursor_           = wndClass_->hCursor;

    // OS にウィンドウクラスを登録
    RegisterClassEx(wndClass_.get());

    // 指定したクライアント領域サイズから、枠を含めたウィンドウ矩形を計算
    RECT wrc = {0, 0, _desc.clientWidth, _desc.clientHeight};
    AdjustWindowRect(&wrc, windowStyle_, false);

    // ウィンドウインスタンスの生成
    DWORD exStyle = backgroundTransparent_ ? WS_EX_LAYERED : 0;
    hwnd_ = CreateWindowEx(
        exStyle,
        wndClass_->lpszClassName,
        wideWindowTitle_.c_str(),
        windowMode_ == WindowMode::WINDOWED ? windowStyle_ : WS_POPUP | WS_VISIBLE,
        CW_USEDEFAULT, // 初期 X 座標
        CW_USEDEFAULT, // 初期 Y 座標
        wrc.right - wrc.left, // 計算された枠込みの幅
        wrc.bottom - wrc.top, // 計算された枠込みの高さ
        nullptr,
        nullptr,
        wndClass_->hInstance,
        nullptr);

    // インスタンスへのポインタを GWLP_USERDATA に格納し、WindowProc で参照可能にする
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ApplyBackgroundTransparency();

    currentDpi_ = GetDpiForWindow(hwnd_);
    dpiScale_   = static_cast<float>(currentDpi_) / static_cast<float>(Config::Window::kDefaultDpi);

    ShowCursor(_desc.showCursor);
    SetCursorClip(_desc.clipCursor);

    if (_desc.iconPath) {
        SetIcon(_desc.iconPath);
    } else if (_desc.iconResourceId > 0) {
        SetIcon(_desc.iconResourceId);
    }

    if (_desc.enableDragDrop) {
        DragAcceptFiles(hwnd_, TRUE);
        dragDropEnabled_ = true;
    }

    // 生成後、実際にウィンドウを可視化
    ShowWindow(hwnd_, SW_NORMAL);
    GetWindowRect(hwnd_, &windowRect_);
    if (windowMode_ != WindowMode::WINDOWED) {
        SetWindowMode(windowMode_);
    }
    RestoreWindowState();
}

/// <summary>
/// ウィンドウを破棄し、登録されたウィンドウクラスを抹消する.
/// </summary>
void WinApp::TerminateGameWindow() {
    SaveWindowState();
    if (dragDropEnabled_ && hwnd_) {
        DragAcceptFiles(hwnd_, FALSE);
        dragDropEnabled_ = false;
    }
    ClipCursor(nullptr);
    ReleaseOwnedIcons();
    ResetCursor();
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;

    UnregisterClass(wndClass_->lpszClassName, wndClass_->hInstance);

    // COM 使用の終了
    CoUninitialize();
}

void WinApp::SetMinWindowSize(int32_t _width, int32_t _height) {
    minWidth_  = std::max(0, _width);
    minHeight_ = std::max(0, _height);
}

void WinApp::SetMaxWindowSize(int32_t _width, int32_t _height) {
    maxWidth_  = std::max(0, _width);
    maxHeight_ = std::max(0, _height);
}

void WinApp::ShowCursor(bool _show) {
    if (cursorVisible_ == _show) {
        return;
    }

    cursorVisible_ = _show;
    if (_show) {
        while (::ShowCursor(TRUE) < 0) {}
    } else {
        while (::ShowCursor(FALSE) >= 0) {}
    }
}

void WinApp::SetCursorClip(bool _clip) {
    cursorClipped_ = _clip;
    if (!_clip) {
        ClipCursor(nullptr);
        return;
    }
    ApplyCursorClip();
}

void WinApp::SetCustomCursor(const wchar_t* _cursorPath) {
    ResetCursor();
    if (_cursorPath == nullptr) {
        return;
    }
    customCursor_ = static_cast<HCURSOR>(LoadImageW(nullptr, _cursorPath, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
}

void WinApp::ResetCursor() {
    if (customCursor_) {
        DestroyCursor(customCursor_);
        customCursor_ = nullptr;
    }
}

void WinApp::SetIcon(const wchar_t* _iconPath) {
    if (_iconPath == nullptr) {
        return;
    }

    ReleaseOwnedIcons();
    iconLarge_ = static_cast<HICON>(LoadImageW(nullptr, _iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
    iconSmall_ = static_cast<HICON>(LoadImageW(nullptr, _iconPath, IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
    ownsIcons_ = true;

    if (iconLarge_) {
        SendMessage(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(iconLarge_));
    }
    if (iconSmall_) {
        SendMessage(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(iconSmall_));
    }
}

void WinApp::SetIcon(int _resourceId) {
    if (_resourceId <= 0) {
        return;
    }

    ReleaseOwnedIcons();
    iconLarge_ = LoadIcon(wndClass_->hInstance, MAKEINTRESOURCE(_resourceId));
    iconSmall_ = static_cast<HICON>(LoadImageW(wndClass_->hInstance, MAKEINTRESOURCE(_resourceId), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    ownsIcons_ = false;

    if (iconLarge_) {
        SendMessage(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(iconLarge_));
    }
    if (iconSmall_) {
        SendMessage(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(iconSmall_));
    }
}

std::vector<DisplayMode> WinApp::EnumerateDisplayModes(int _monitorIndex) {
    DISPLAY_DEVICEW device{};
    device.cb = sizeof(device);

    std::wstring deviceName;
    for (DWORD index = 0, activeIndex = 0; EnumDisplayDevicesW(nullptr, index, &device, 0); ++index) {
        if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) == 0) {
            device.cb = sizeof(device);
            continue;
        }
        if (static_cast<int>(activeIndex) == _monitorIndex) {
            deviceName = device.DeviceName;
            break;
        }
        ++activeIndex;
        device.cb = sizeof(device);
    }

    const wchar_t* targetDevice = deviceName.empty() ? nullptr : deviceName.c_str();
    std::vector<DisplayMode> modes;
    std::set<std::tuple<int32_t, int32_t, int32_t>> uniqueModes;
    DEVMODEW devMode{};
    devMode.dmSize = sizeof(devMode);
    for (DWORD modeIndex = 0; EnumDisplaySettingsW(targetDevice, modeIndex, &devMode); ++modeIndex) {
        auto key = std::make_tuple(
            static_cast<int32_t>(devMode.dmPelsWidth),
            static_cast<int32_t>(devMode.dmPelsHeight),
            static_cast<int32_t>(devMode.dmDisplayFrequency));
        if (uniqueModes.insert(key).second) {
            modes.push_back(DisplayMode{std::get<0>(key), std::get<1>(key), std::get<2>(key)});
        }
    }

    std::sort(modes.begin(), modes.end(), [](const DisplayMode& lhs, const DisplayMode& rhs) {
        if (lhs.width != rhs.width) {
            return lhs.width < rhs.width;
        }
        if (lhs.height != rhs.height) {
            return lhs.height < rhs.height;
        }
        return lhs.refreshRate < rhs.refreshRate;
    });
    return modes;
}

void WinApp::ChangeResolution(int32_t _width, int32_t _height) {
    clientWidth_  = std::max(1, _width);
    clientHeight_ = std::max(1, _height);
    windowSize_   = Vec2f(float(clientWidth_), float(clientHeight_));
    aspectRatio_  = windowSize_[X] / windowSize_[Y];

    if (windowMode_ == WindowMode::BORDERLESS_FULLSCREEN || windowMode_ == WindowMode::EXCLUSIVE_FULLSCREEN) {
        SetWindowMode(windowMode_);
        return;
    }

    RECT rect{0, 0, clientWidth_, clientHeight_};
    DWORD style = static_cast<DWORD>(GetWindowLongPtr(hwnd_, GWL_STYLE));
    AdjustWindowRect(&rect, style, FALSE);
    SetWindowPos(hwnd_, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void WinApp::SetWindowMode(WindowMode _mode) {
    if (hwnd_ == nullptr) {
        windowMode_ = _mode;
        return;
    }

    bool wasFullscreen = IsFullscreen();
    if (!wasFullscreen && (_mode == WindowMode::BORDERLESS_FULLSCREEN || _mode == WindowMode::EXCLUSIVE_FULLSCREEN)) {
        GetWindowRect(hwnd_, &windowRect_);
    }

    windowMode_ = _mode;

    if (_mode == WindowMode::WINDOWED) {
        SetWindowLongPtr(hwnd_, GWL_STYLE, static_cast<LONG_PTR>(windowStyle_ | WS_VISIBLE));
        SetWindowPos(hwnd_, alwaysOnTop_ ? HWND_TOPMOST : HWND_NOTOPMOST,
            windowRect_.left, windowRect_.top,
            windowRect_.right - windowRect_.left,
            windowRect_.bottom - windowRect_.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        return;
    }

    if (_mode == WindowMode::BORDERLESS_WINDOWED) {
        SetWindowLongPtr(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwnd_, alwaysOnTop_ ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        return;
    }

    RECT monitorRect = GetMonitorRect(targetMonitorIndex_);
    SetWindowLongPtr(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(hwnd_, alwaysOnTop_ ? HWND_TOPMOST : HWND_TOP,
        monitorRect.left, monitorRect.top,
        monitorRect.right - monitorRect.left,
        monitorRect.bottom - monitorRect.top,
        SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

void WinApp::SaveWindowState() {
    if (hwnd_ == nullptr) {
        return;
    }

    RECT rect{};
    GetWindowRect(hwnd_, &rect);

    SerializedField<int32_t> posX{"Settings", "WindowState", "PosX", rect.left};
    SerializedField<int32_t> posY{"Settings", "WindowState", "PosY", rect.top};
    SerializedField<int32_t> width{"Settings", "WindowState", "Width", clientWidth_};
    SerializedField<int32_t> height{"Settings", "WindowState", "Height", clientHeight_};
    SerializedField<int32_t> mode{"Settings", "WindowState", "Mode", static_cast<int32_t>(windowMode_)};
    SerializedField<int32_t> monitorIndex{"Settings", "WindowState", "MonitorIndex", GetCurrentMonitorIndex()};
    SerializedField<bool> backgroundTransparent{"Settings", "WindowState", "BackgroundTransparent", backgroundTransparent_};
    SerializedField<int32_t> backgroundAlpha{"Settings", "WindowState", "BackgroundAlpha", static_cast<int32_t>(backgroundAlpha_)};
    SerializedField<int32_t> transparencyColorKey{"Settings", "WindowState", "TransparencyColorKey", static_cast<int32_t>(transparencyColorKey_)};
    SerializedField<bool> useTransparencyColorKey{"Settings", "WindowState", "UseTransparencyColorKey", useTransparencyColorKey_};

    posX.SetValue(rect.left);
    posY.SetValue(rect.top);
    width.SetValue(clientWidth_);
    height.SetValue(clientHeight_);
    mode.SetValue(static_cast<int32_t>(windowMode_));
    monitorIndex.SetValue(GetCurrentMonitorIndex());
    backgroundTransparent.SetValue(backgroundTransparent_);
    backgroundAlpha.SetValue(static_cast<int32_t>(backgroundAlpha_));
    transparencyColorKey.SetValue(static_cast<int32_t>(transparencyColorKey_));
    useTransparencyColorKey.SetValue(useTransparencyColorKey_);
    GlobalVariables::GetInstance()->SaveFile("Settings", "WindowState");
}

bool WinApp::RestoreWindowState() {
    auto* scene = GlobalVariables::GetInstance()->GetScene("Settings");
    if (scene == nullptr) {
        return false;
    }

    auto groupItr = scene->find("WindowState");
    if (groupItr == scene->end()) {
        return false;
    }

    const auto& group = groupItr->second;
    if (!group.contains("PosX") || !group.contains("PosY") || !group.contains("Width")
        || !group.contains("Height") || !group.contains("Mode") || !group.contains("MonitorIndex")) {
        return false;
    }

    SerializedField<int32_t> posX{"Settings", "WindowState", "PosX"};
    SerializedField<int32_t> posY{"Settings", "WindowState", "PosY"};
    SerializedField<int32_t> width{"Settings", "WindowState", "Width"};
    SerializedField<int32_t> height{"Settings", "WindowState", "Height"};
    SerializedField<int32_t> mode{"Settings", "WindowState", "Mode"};
    SerializedField<int32_t> monitorIndex{"Settings", "WindowState", "MonitorIndex"};
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();
    bool* backgroundTransparent = globalVariables->AddValue<bool>(
        "Settings", "WindowState", "BackgroundTransparent", backgroundTransparent_);
    int32_t* backgroundAlpha = globalVariables->AddValue<int32_t>(
        "Settings", "WindowState", "BackgroundAlpha", static_cast<int32_t>(backgroundAlpha_));
    int32_t* transparencyColorKey = globalVariables->AddValue<int32_t>(
        "Settings", "WindowState", "TransparencyColorKey", static_cast<int32_t>(transparencyColorKey_));
    bool* useTransparencyColorKey = globalVariables->AddValue<bool>(
        "Settings", "WindowState", "UseTransparencyColorKey", useTransparencyColorKey_);

    POINT topLeft{*posX.GetValue(), *posY.GetValue()};
    auto monitors = EnumerateMonitors();
    bool isInsideAnyMonitor = std::any_of(monitors.begin(), monitors.end(), [&topLeft](const MonitorInfo& monitor) {
        return PtInRect(&monitor.monitorArea, topLeft) != FALSE;
    });
    if (!isInsideAnyMonitor && !monitors.empty()) {
        return false;
    }

    targetMonitorIndex_ = std::max(0, *monitorIndex.GetValue());
    backgroundTransparent_ = *backgroundTransparent;
    backgroundAlpha_ = static_cast<BYTE>(std::clamp(*backgroundAlpha, 0, 255));
    transparencyColorKey_ = static_cast<COLORREF>(*transparencyColorKey);
    useTransparencyColorKey_ = *useTransparencyColorKey;
    ApplyBackgroundTransparency();
    ChangeResolution(*width.GetValue(), *height.GetValue());
    windowRect_ = RECT{*posX.GetValue(), *posY.GetValue(), *posX.GetValue() + *width.GetValue(), *posY.GetValue() + *height.GetValue()};
    SetWindowMode(static_cast<WindowMode>(*mode.GetValue()));
    if (windowMode_ == WindowMode::WINDOWED || windowMode_ == WindowMode::BORDERLESS_WINDOWED) {
        SetWindowPos(hwnd_, nullptr, *posX.GetValue(), *posY.GetValue(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    return true;
}

void WinApp::SetDropCallback(const DropCallback& _callback) {
    dropCallback_ = _callback;
}

void WinApp::ClearDropCallback() {
    dropCallback_ = nullptr;
}

void WinApp::SetWindowTitle(const wchar_t* _title) {
    if (_title == nullptr) {
        return;
    }
    if (wideWindowTitle_ == _title) {
        return;
    }
    wideWindowTitle_ = _title;
    if (hwnd_) {
        SetWindowTextW(hwnd_, wideWindowTitle_.c_str());
    }
}

std::vector<MonitorInfo> WinApp::EnumerateMonitors() {
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM data) -> BOOL {
            auto* result = reinterpret_cast<std::vector<MonitorInfo>*>(data);
            MONITORINFOEXW info{};
            info.cbSize = sizeof(info);
            if (GetMonitorInfoW(monitor, &info) == FALSE) {
                return TRUE;
            }

            MonitorInfo monitorInfo{};
            monitorInfo.name        = info.szDevice;
            monitorInfo.workArea    = info.rcWork;
            monitorInfo.monitorArea = info.rcMonitor;
            monitorInfo.isPrimary   = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
            result->push_back(monitorInfo);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&monitors));
    return monitors;
}

void WinApp::SetTargetMonitor(int _index) {
    auto monitors = EnumerateMonitors();
    if (monitors.empty()) {
        targetMonitorIndex_ = 0;
        return;
    }
    targetMonitorIndex_ = std::clamp(_index, 0, static_cast<int>(monitors.size()) - 1);
    if (IsFullscreen()) {
        SetWindowMode(windowMode_);
    }
}

int WinApp::GetCurrentMonitorIndex() const {
    if (hwnd_ == nullptr) {
        return targetMonitorIndex_;
    }

    HMONITOR current = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    auto monitors    = EnumerateMonitors();
    for (int index = 0; index < static_cast<int>(monitors.size()); ++index) {
        HMONITOR monitor = MonitorFromRect(&monitors[index].monitorArea, MONITOR_DEFAULTTONULL);
        if (monitor == current) {
            return index;
        }
    }
    return targetMonitorIndex_;
}

void WinApp::SetAlwaysOnTop(bool _enable) {
    alwaysOnTop_ = _enable;
    if (hwnd_ == nullptr) {
        return;
    }
    SetWindowPos(hwnd_, _enable ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);
}

void WinApp::SetBackgroundTransparency(bool _enable) {
    backgroundTransparent_ = _enable;
    ApplyBackgroundTransparency();
}

void WinApp::SetWindowOpacity(BYTE _alpha) {
    backgroundAlpha_ = _alpha;
    ApplyBackgroundTransparency();
}

void WinApp::SetTransparencyColorKey(COLORREF _colorKey, bool _enable) {
    transparencyColorKey_ = _colorKey;
    useTransparencyColorKey_ = _enable;
    ApplyBackgroundTransparency();
}

/// <summary>
/// OS のメッセージキューを監視し、メッセージがあれば処理する.
/// </summary>
/// <returns>WM_QUIT を受け取った場合は true, それ以外は false</returns>
bool WinApp::ProcessMessage() {
    MSG msg{}; // メッセージ

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // メッセージがあれば取り出す
        TranslateMessage(&msg); // 仮想キーメッセージを文字メッセージに変換
        DispatchMessage(&msg); // ウィンドウプロシージャへ転送
    }

    if (msg.message == WM_QUIT) { // アプリケーション終了メッセージの検知
        return true;
    }

    return false;
}

/// <summary>
/// 現在このウィンドウが最前面 (フォーカスされている) かをチェックし状態を更新する.
/// </summary>
void WinApp::UpdateActivity() {
    isActive_ = GetForegroundWindow() == hwnd_;
}

/// <summary>
/// 外部プロセスを実行し、その終了を待機するヘルパー関数.
/// 標準出力と標準エラーをパイプ経由で取得し、必要に応じてログ出力等に利用可能.
/// </summary>
/// <param name="_command">実行するコマンドライン文字列</param>
/// <param name="_currentDirectory">プロセスの作業ディレクトリ (nullptr の場合はカレント)</param>
/// <returns>プロセスの生成に成功した場合は true</returns>
bool OriGine::RunProcessAndWait(const std::string& _command, const char* _currentDirectory) {
    std::string currentDirStr;
    if (!_currentDirectory) {
        std::filesystem::path current = std::filesystem::current_path();
        currentDirStr                 = current.string();
        _currentDirectory             = currentDirStr.c_str();
    }

    // パイプの作成 (子プロセスの出力を親プロセスで読み取るため)
    SECURITY_ATTRIBUTES saAttr  = {};
    saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle       = TRUE; // ハンドル継承を許可
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE hRead = nullptr, hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        fprintf(stderr, "Failed to create pipe\n");
        return false;
    }
    // 親プロセス側で読み取り専用とする (継承設定の解除)
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {sizeof(si)};
    si.dwFlags      = STARTF_USESTDHANDLES;
    si.hStdOutput   = hWrite;
    si.hStdError    = hWrite;
    si.hStdInput    = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;

    std::vector<char> cmdLine(_command.size() + 1);
    strcpy_s(cmdLine.data(), cmdLine.size(), _command.c_str());

    // プロセスの生成
    if (!CreateProcessA(
            nullptr,
            cmdLine.data(),
            nullptr,
            nullptr,
            TRUE, // パイプ経由の入出力を継承するため TRUE
            0,
            nullptr,
            _currentDirectory,
            &si,
            &pi)) {
        DWORD error = GetLastError();
        char msgBuf[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            msgBuf,
            sizeof(msgBuf),
            nullptr);
        fprintf(stderr, "CreateProcess failed with error code %lu: %s\n", error, msgBuf);
        LOG_ERROR("CreateProcess failed with error code {}: {}", error, msgBuf);
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }

    // 書き込み側ハンドルは子プロセスに渡したので、親側では閉じる
    CloseHandle(hWrite);

    // 子プロセスの出力を読み取るループ
    std::ostringstream oss;
    char buffer[4096];
    DWORD bytesRead = 0;
    BOOL success    = FALSE;
    while (true) {
        success = ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr);
        if (!success || bytesRead == 0)
            break;
        buffer[bytesRead] = '\0';
        oss << buffer;
    }
    CloseHandle(hRead);

    // プロセスの終了を完全に待つ
    WaitForSingleObject(pi.hProcess, INFINITE);

    // プロセスの終了コードを取得
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    if (exitCode != 0) {
        fprintf(stderr, "Process exited with code %lu\n", exitCode);
        LOG_ERROR("Process exited with code {}", exitCode);

        std::string output = oss.str();
        if (!output.empty()) {
            fprintf(stderr, "Process output:\n%s\n", output.c_str());
            LOG_ERROR("Process output:\n{}", output);
        }
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode == 0;
}
