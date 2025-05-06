#include "winApp/WinApp.h"

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

    case WM_SIZE: // ウィンドウサイズが変更された
        if (wparam != SIZE_MINIMIZED) { // 最小化時は無視
            // ウィンドウに関連付けられた WinApp インスタンスを取得
            WinApp* pThis = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (pThis == nullptr) {
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            pThis->isReSized_    = true; // リサイズフラグを立てる
            pThis->clientWidth_  = LOWORD(lparam); // ウィンドウの幅
            pThis->clientHeight_ = HIWORD(lparam); // ウィンドウの高さ
            pThis->windowSize_   = Vec2f(float(pThis->clientWidth_), float(pThis->clientHeight_));
        }
        return 0;
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

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // メッセージがある？
        TranslateMessage(&msg); // キー入力メッセージの処理
        DispatchMessage(&msg); // ウィンドウプロシージャにメッセージを送る
    }

    if (msg.message == WM_QUIT) { // 終了メッセージが来たらループを抜ける
        return true;
    }

    return false;
}
