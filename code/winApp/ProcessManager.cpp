#include "ProcessManager.h"

#include <Psapi.h>
#include <shellapi.h>

#include <algorithm>

#include "logger/Logger.h"

#pragma comment(lib, "Psapi.lib")

namespace OriGine {

namespace {

/// <summary>
/// EnumWindowProc からウィンドウ列挙結果の格納先とフォアグラウンドウィンドウを参照するための文脈情報.
/// </summary>
struct EnumContext {
    std::vector<WindowProcessInfo>* results;
    HWND foregroundHwnd;
};

/// <summary>
/// EnumWindows に渡すコールバック. 実際に意味のある（可視・非子・非ツール）ウィンドウのみを抽出し、
/// 所有プロセスの実行ファイル名を解決して results に追加する.
/// </summary>
BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE; // 非表示ウィンドウは除外

    wchar_t title[512] = {};
    int titleLen = GetWindowTextW(hwnd, title, 512);
    if (titleLen == 0) return TRUE; // タイトルなしは除外

    DWORD style = static_cast<DWORD>(GetWindowLongW(hwnd, GWL_STYLE));
    if (style & WS_CHILD) return TRUE; // 子ウィンドウは除外

    DWORD exStyle = static_cast<DWORD>(GetWindowLongW(hwnd, GWL_EXSTYLE));
    if (exStyle & WS_EX_TOOLWINDOW) return TRUE; // ツールウィンドウ（タスクバー非表示用）は除外

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return TRUE;

    // 実行ファイル名解決のため、必要最小限の権限でプロセスをオープン
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return TRUE;

    wchar_t exePath[MAX_PATH] = {};
    DWORD exePathSize = MAX_PATH;
    std::wstring exeName;
    if (QueryFullProcessImageNameW(hProcess, 0, exePath, &exePathSize)) {
        std::wstring fullPath(exePath);
        auto pos = fullPath.find_last_of(L"\\/");
        exeName = (pos != std::wstring::npos) ? fullPath.substr(pos + 1) : fullPath; // フルパスから実行ファイル名のみ抽出
    }
    CloseHandle(hProcess);

    if (exeName.empty()) return TRUE;

    auto* ctx = reinterpret_cast<EnumContext*>(lParam);

    WindowProcessInfo info;
    info.processId = pid;
    info.hwnd = hwnd;
    info.exeName = std::move(exeName);
    info.windowTitle = title;
    info.isForeground = (hwnd == ctx->foregroundHwnd);

    ctx->results->push_back(std::move(info));
    return TRUE; // 列挙を継続
}

/// <summary>
/// ワイド文字列（UTF-16）を UTF-8 の std::string へ変換する.
/// WideCharToMultiByte を必要バイト数取得用と実変換用の2段階で呼び出す.
/// </summary>
std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return {};
    // 1回目: 変換後に必要なバイト数を取得
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
        static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size, '\0');
    // 2回目: 実際の変換を実施
    WideCharToMultiByte(CP_UTF8, 0, wide.data(),
        static_cast<int>(wide.size()), result.data(), size, nullptr, nullptr);
    return result;
}

} // namespace

/// <summary>
/// 現在デスクトップ上に存在する可視ウィンドウを列挙する.
/// 子ウィンドウやツールウィンドウは対象外.
/// </summary>
/// <returns>列挙されたウィンドウ情報のリスト</returns>
std::vector<WindowProcessInfo> ProcessManager::EnumerateWindows() {
    std::vector<WindowProcessInfo> results;
    // EnumWindows は自前のコールバックしか呼べないため、格納先とフォアグラウンドHWNDを
    // EnumContext にまとめ、LPARAM 経由でコールバック(EnumWindowProc)へ渡す
    EnumContext ctx{&results, ::GetForegroundWindow()};
    ::EnumWindows(EnumWindowProc, reinterpret_cast<LPARAM>(&ctx));
    return results;
}

/// <summary>
/// 現在フォアグラウンドにあるウィンドウの情報を取得する.
/// </summary>
/// <returns>フォアグラウンドウィンドウの情報</returns>
WindowProcessInfo ProcessManager::GetForegroundApp() {
    HWND fg = ::GetForegroundWindow();
    // フォアグラウンドウィンドウが存在しない(全ウィンドウが最小化されている等)場合は
    // これ以上取得できる情報が無いため、空の情報を返して抜ける
    if (!fg) return {};

    wchar_t title[512] = {};
    GetWindowTextW(fg, title, 512);

    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);

    WindowProcessInfo info;
    info.processId = pid;
    info.hwnd = fg;
    info.windowTitle = title;
    info.isForeground = true;

    // 実行ファイル名の解決にはプロセスハンドルが必要なため取得する。
    // 取得できなくても hwnd/pid/title 等は既に確定しているため致命的ではなく、exeName が空のまま返す
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        wchar_t exePath[MAX_PATH] = {};
        DWORD exePathSize = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, exePath, &exePathSize)) {
            std::wstring fullPath(exePath);
            auto pos = fullPath.find_last_of(L"\\/");
            info.exeName = (pos != std::wstring::npos) ? fullPath.substr(pos + 1) : fullPath;
        }
        // OpenProcess で取得したハンドルは使い終わったら必ず閉じる。
        // ここで閉じ忘れるとプロセスを呼ぶたびにハンドルリークが蓄積する
        CloseHandle(hProcess);
    }
    return info;
}

/// <summary>
/// 指定したパスの実行ファイル・ドキュメントを起動する.
/// </summary>
/// <param name="path">実行ファイルのパス</param>
/// <param name="args">起動時に渡すコマンドライン引数</param>
/// <returns>起動に成功したか</returns>
bool ProcessManager::LaunchProcess(const std::wstring& path, const std::wstring& args) {
    HINSTANCE result = ShellExecuteW(
        nullptr, L"open", path.c_str(),
        args.empty() ? nullptr : args.c_str(),
        nullptr, SW_SHOWNORMAL);

    // ShellExecute は成功時は実行に使われたインスタンスハンドルを返すが、失敗時は
    // HINSTANCE を装ったエラーコード(32以下)を返す、という古いWin16由来の特殊な規約を持つ
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        LOG_ERROR("ShellExecute failed for: {}", WideToUtf8(path));
        return false;
    }
    return true;
}

/// <summary>
/// 指定したウィンドウを最前面（フォアグラウンド）へ表示する.
/// </summary>
/// <param name="hwnd">対象ウィンドウハンドル</param>
/// <returns>操作に成功したか</returns>
bool ProcessManager::BringToForeground(HWND hwnd) {
    // ハンドルが既に無効(ウィンドウが閉じられた等)なら、以降の操作は無意味なので早期return
    if (!IsWindow(hwnd)) return false;

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE); // 最小化されている場合は元のサイズへ復元
    }

    DWORD foregroundThread = GetWindowThreadProcessId(::GetForegroundWindow(), nullptr);
    DWORD targetThread = GetWindowThreadProcessId(hwnd, nullptr);

    // Windows の制限により、フォアグラウンドスレッド以外からの SetForegroundWindow は無視されることがあるため、
    // 入力キューを一時的にアタッチしてから前面化し、直後にデタッチして元の状態へ戻す
    if (foregroundThread != targetThread) {
        AttachThreadInput(foregroundThread, targetThread, TRUE);
        SetForegroundWindow(hwnd);
        // アタッチしたままだと2つのスレッドの入力状態が結合されたままになり、
        // 対象ウィンドウ操作後もフォーカスやキー入力の挙動に影響するため、必ずデタッチして元へ戻す
        AttachThreadInput(foregroundThread, targetThread, FALSE);
    } else {
        SetForegroundWindow(hwnd);
    }

    return true;
}

/// <summary>
/// 指定したウィンドウへ終了メッセージ（WM_CLOSE）を送信する.
/// </summary>
/// <param name="hwnd">対象ウィンドウハンドル</param>
/// <returns>メッセージ送信に成功したか</returns>
bool ProcessManager::CloseWindow(HWND hwnd) {
    // ハンドルが既に無効なら送信対象が無いため早期return
    if (!IsWindow(hwnd)) return false;
    // PostMessage は非同期であり、送信できたかのみを返す。対象アプリが実際に終了処理を
    // 行うかは相手のウィンドウプロシージャ次第（未応答なら強制終了させたい場合は TerminateApp を使う）
    return PostMessageW(hwnd, WM_CLOSE, 0, 0) != 0;
}

/// <summary>
/// 指定したプロセスIDのプロセスを強制終了する.
/// </summary>
/// <param name="processId">対象プロセスID</param>
/// <returns>終了に成功したか</returns>
bool ProcessManager::TerminateApp(DWORD processId) {
    // TerminateProcess の実行に必要な最小権限(PROCESS_TERMINATE)のみでハンドルを取得する
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!hProcess) {
        // プロセスが既に終了している、あるいは権限不足でハンドルを取得できない場合はここで諦める
        LOG_ERROR("OpenProcess failed for PID {}", processId);
        return false;
    }
    BOOL result = ::TerminateProcess(hProcess, 1);
    // OpenProcess で取得したハンドルは結果に関わらず必ず閉じる（閉じ忘れはハンドルリークに直結する）
    CloseHandle(hProcess);
    return result != 0;
}

/// <summary>
/// ウィンドウ情報のリストを、人が読める形式のテキストへ整形する.
/// </summary>
/// <param name="windows">整形対象のウィンドウ情報リスト</param>
/// <returns>整形されたテキスト</returns>
std::string ProcessManager::FormatAsText(const std::vector<WindowProcessInfo>& windows) {
    std::string text;
    text.reserve(windows.size() * 80);

    for (const auto& w : windows) {
        if (w.isForeground) {
            text += "[foreground] ";
        }
        text += WideToUtf8(w.exeName);
        text += " - ";
        text += WideToUtf8(w.windowTitle);
        text += "\n";
    }
    return text;
}

} // namespace OriGine
