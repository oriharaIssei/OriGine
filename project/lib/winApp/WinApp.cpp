#include "winApp/WinApp.h"

/// stl
#include <filesystem>
#include <iostream>
#include <sstream>

#include <vector>

#ifdef _DEBUG
#include "imgui/imgui.h"
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef _DEBUG
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }
#endif // _DEBUG

    // ウィンドウに起こったイベントに対して行う処理
    switch (msg) {
    case WM_DESTROY: // ウィンドウが破棄された
        PostQuitMessage(0); // OSに対して、アプリの終了を伝える
        return 0;

    case WM_SIZING: {
        WinApp* pThis = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (pThis && pThis->windowResizeMode_ == WindowResizeMode::FIXED_ASPECT) {
            RECT* rect   = reinterpret_cast<RECT*>(lparam);
            int width    = rect->right - rect->left;
            int height   = rect->bottom - rect->top;
            float aspect = pThis->aspectRatio_;

            switch (wparam) {
            case WMSZ_RIGHT: {
                // 右端のみを動かす
                width        = rect->right - rect->left;
                height       = int(width / aspect);
                rect->bottom = rect->top + height;
                break;
            }
            case WMSZ_LEFT: {
                // 左端のみを動かす
                width     = rect->right - rect->left;
                height    = int(width / aspect);
                rect->top = rect->bottom - height;
                break;
            }
            case WMSZ_TOP: {
                // 上端のみを動かす
                height      = rect->bottom - rect->top;
                width       = int(height * aspect);
                rect->right = rect->left + width;
                break;
            }
            case WMSZ_BOTTOM: {
                // 下端のみを動かす
                height      = rect->bottom - rect->top;
                width       = int(height * aspect);
                rect->right = rect->left + width;
                break;
            }
            case WMSZ_TOPRIGHT: {
                // 右上を動かす
                width     = rect->right - rect->left;
                height    = int(width / aspect);
                rect->top = rect->bottom - height;
                break;
            }
            case WMSZ_TOPLEFT: {
                // 左上を動かす
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
        WinApp* pThis = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (pThis == nullptr) {
            return DefWindowProc(hwnd, msg, wparam, lparam); // デフォルトの処理
        }
        if (wparam != SIZE_MINIMIZED) {
            pThis->isReSized_ = true;

            pThis->clientWidth_  = LOWORD(lparam);
            pThis->clientHeight_ = HIWORD(lparam);
            pThis->windowSize_   = Vec2f(float(pThis->clientWidth_), float(pThis->clientHeight_));
        }

        break;
    }
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_IME_KEYDOWN:
    case WM_IME_KEYUP:
        if ((wparam == VK_MENU) || wparam == VK_F10) {

            // SetUseHookWinProcReturnValue(TRUE);

            // AltとF10を無視する.
            return 0;
        }
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam); // デフォルトの処理
}

WinApp::~WinApp() {
    CloseWindow(hwnd_);
}

void WinApp::CreateGameWindow(const wchar_t* title, UINT windowStyle, int32_t clientWidth, int32_t clientHeight) {
    // COM初期化
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    wideWindowTitle_ = title;
    windowStyle_     = windowStyle;

    clientWidth_  = clientWidth;
    clientHeight_ = clientHeight;
    windowSize_   = Vec2f(float(clientWidth_), float(clientHeight_));
    aspectRatio_  = windowSize_[X] / windowSize_[Y];

    // ウィンドウクラスの初期化
    wndClass_                = std::make_unique<WNDCLASSEX>();
    wndClass_->cbSize        = sizeof(WNDCLASSEX);
    wndClass_->lpfnWndProc   = (WNDPROC)WindowProc; // ウィンドウプロシージャ
    wndClass_->lpszClassName = title; // ウィンドウクラス名
    wndClass_->hInstance     = GetModuleHandle(nullptr); // ウィンドウハンドル
    wndClass_->hCursor       = LoadCursor(NULL, IDC_ARROW); // カーソル指定

    RegisterClassEx(wndClass_.get());

    // ウィンドウサイズ{ X座標 Y座標 横幅 縦幅 }
    RECT wrc = {0, 0, clientWidth, clientHeight};
    AdjustWindowRect(&wrc, windowStyle_, false);

    // ウィンドウオブジェクトの生成
    hwnd_ = CreateWindow(
        wndClass_->lpszClassName, // クラス名
        wideWindowTitle_.c_str(), // タイトルバーの文字
        windowStyle_, // タイトルバーと境界線があるウィンドウ
        CW_USEDEFAULT, // 表示X座標（OSに任せる）
        CW_USEDEFAULT, // 表示Y座標（OSに任せる）
        wrc.right - wrc.left, // ウィンドウ横幅
        wrc.bottom - wrc.top, // ウィンドウ縦幅
        nullptr, // 親ウィンドウハンドル
        nullptr, // メニューハンドル
        wndClass_->hInstance, // 呼び出しアプリケーションハンドル
        nullptr); // オプション

    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // ウィンドウ表示
    ShowWindow(hwnd_, SW_NORMAL);
}

void WinApp::TerminateGameWindow() {
    // ウィンドウクラスを登録解除
    UnregisterClass(wndClass_->lpszClassName, wndClass_->hInstance);

    // COM 終了
    CoUninitialize();
}

bool WinApp::ProcessMessage() {
    MSG msg{}; // メッセージ

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // メッセージがある？
        TranslateMessage(&msg); // キー入力メッセージの処理
        DispatchMessage(&msg); // ウィンドウプロシージャにメッセージを送る
    }

    if (msg.message == WM_QUIT) { // 終了メッセージが来たらループを抜ける
        return true;
    }

    return false;
}

void WinApp::UpdateActivity() {
    isActive_ = GetForegroundWindow() == hwnd_;
}


bool RunProcessAndWait(const std::string& command, const char* _currentDirectory) {
    std::string currentDirStr;
    if (!_currentDirectory) {
        std::filesystem::path current = std::filesystem::current_path();
        currentDirStr                 = current.string();
        _currentDirectory             = currentDirStr.c_str();
    }

    // パイプ作成（標準出力・標準エラー用）
    SECURITY_ATTRIBUTES saAttr  = {};
    saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle       = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE hRead = nullptr, hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        fprintf(stderr, "Failed to create pipe\n");
        return false;
    }
    // 親プロセスで読み取り側は継承しない
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {sizeof(si)};
    si.dwFlags      = STARTF_USESTDHANDLES;
    si.hStdOutput   = hWrite;
    si.hStdError    = hWrite;
    si.hStdInput    = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;

    std::vector<char> cmdLine(command.size() + 1);
    strcpy_s(cmdLine.data(), cmdLine.size(), command.c_str());

    if (!CreateProcessA(
            nullptr,
            cmdLine.data(),
            nullptr,
            nullptr,
            TRUE, // パイプを継承するためTRUE
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
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }

    // 子プロセスでのみ使うので親で書き込み側を閉じる
    CloseHandle(hWrite);

    // 出力を読み取る
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

    // プロセスの終了を待機
    WaitForSingleObject(pi.hProcess, INFINITE);

    // プロセスの終了コードを取得
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    if (exitCode != 0) {
        fprintf(stderr, "Process exited with code %lu\n", exitCode);
        std::string output = oss.str();
        if (!output.empty()) {
            fprintf(stderr, "Process output:\n%s\n", output.c_str());
        }
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode == 0;
}
