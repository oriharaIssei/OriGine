#include "winApp/WinApp.h"

/// stl
#include <filesystem>
#include <iostream>
#include <sstream>

#include <vector>

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
        }

        break;
    }
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
        if (_wparam == VK_F11 && pThis) {
            pThis->ToggleFullscreen(!pThis->isFullscreen_);
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
    CloseWindow(hwnd_);
}
/// <summary>
/// フルスクリーンとウィンドウモードの動的切り替え.
/// フルスクリーン時は WS_POPUP を使用して画面全体に広げ、ウィンドウモード時は WS_OVERLAPPEDWINDOW に戻す.
/// </summary>
void WinApp::ToggleFullscreen(bool _enable) {
    if (_enable && !isFullscreen_) {
        // 現在のウィンドウ位置とサイズを退避
        GetWindowRect(hwnd_, &windowRect_);

        // ボーダーレスポップアップスタイルに変更
        SetWindowLong(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);

        // プライマリモニタの解像度を取得してウィンドウを最大化
        int width  = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(hwnd_, HWND_TOP, 0, 0, width, height,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        isFullscreen_ = true;
    } else if (!_enable && isFullscreen_) {
        // 通常のウィンドウスタイル (タイトルバーあり等) に戻す
        SetWindowLong(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

        // 退避しておいた位置とサイズを復元
        SetWindowPos(hwnd_, HWND_TOP,
            windowRect_.left, windowRect_.top,
            windowRect_.right - windowRect_.left,
            windowRect_.bottom - windowRect_.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        isFullscreen_ = false;
    }
};

/// <summary>
/// ゲーム用ウィンドウを生成し、表示を開始する.
/// COM の初期化、ウィンドウクラスの登録、CreateWindowAPI の呼び出しを実施する.
/// </summary>
void WinApp::CreateGameWindow(const wchar_t* _title, UINT _windowStyle, int32_t _clientWidth, int32_t _clientHeight) {
    // COM (Component Object Model) の初期化. DirectX や WIC 等で必要.
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    wideWindowTitle_ = _title;
    windowStyle_     = _windowStyle;

    clientWidth_  = _clientWidth;
    clientHeight_ = _clientHeight;
    windowSize_   = Vec2f(float(_clientWidth), float(_clientHeight));
    aspectRatio_  = windowSize_[X] / windowSize_[Y];

    // ウィンドウクラス (WNDCLASSEX) の構造体設定
    wndClass_                = std::make_unique<WNDCLASSEX>();
    wndClass_->cbSize        = sizeof(WNDCLASSEX);
    wndClass_->lpfnWndProc   = (WNDPROC)WindowProc; // コールバック関数の指定
    wndClass_->lpszClassName = _title; // 識別用のクラス名
    wndClass_->hInstance     = GetModuleHandle(nullptr); // 実行インスタンス
    wndClass_->hCursor       = LoadCursor(NULL, IDC_ARROW); // デフォルトカーソル

    // OS にウィンドウクラスを登録
    RegisterClassEx(wndClass_.get());

    // 指定したクライアント領域サイズから、枠を含めたウィンドウ矩形を計算
    RECT wrc = {0, 0, _clientWidth, _clientHeight};
    AdjustWindowRect(&wrc, _windowStyle, false);

    // ウィンドウインスタンスの生成
    hwnd_ = CreateWindow(
        wndClass_->lpszClassName,
        wideWindowTitle_.c_str(),
        _windowStyle,
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

    // 生成後、実際にウィンドウを可視化
    ShowWindow(hwnd_, SW_NORMAL);
}

/// <summary>
/// ウィンドウを破棄し、登録されたウィンドウクラスを抹消する.
/// </summary>
void WinApp::TerminateGameWindow() {
    UnregisterClass(wndClass_->lpszClassName, wndClass_->hInstance);

    // COM 使用の終了
    CoUninitialize();
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
