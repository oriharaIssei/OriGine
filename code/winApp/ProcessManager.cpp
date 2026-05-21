#include "ProcessManager.h"

#include <Psapi.h>
#include <shellapi.h>

#include <algorithm>

#include "logger/Logger.h"

#pragma comment(lib, "Psapi.lib")

namespace OriGine {

namespace {

struct EnumContext {
    std::vector<WindowProcessInfo>* results;
    HWND foregroundHwnd;
};

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;

    wchar_t title[512] = {};
    int titleLen = GetWindowTextW(hwnd, title, 512);
    if (titleLen == 0) return TRUE;

    DWORD style = static_cast<DWORD>(GetWindowLongW(hwnd, GWL_STYLE));
    if (style & WS_CHILD) return TRUE;

    DWORD exStyle = static_cast<DWORD>(GetWindowLongW(hwnd, GWL_EXSTYLE));
    if (exStyle & WS_EX_TOOLWINDOW) return TRUE;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return TRUE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return TRUE;

    wchar_t exePath[MAX_PATH] = {};
    DWORD exePathSize = MAX_PATH;
    std::wstring exeName;
    if (QueryFullProcessImageNameW(hProcess, 0, exePath, &exePathSize)) {
        std::wstring fullPath(exePath);
        auto pos = fullPath.find_last_of(L"\\/");
        exeName = (pos != std::wstring::npos) ? fullPath.substr(pos + 1) : fullPath;
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
    return TRUE;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
        static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(),
        static_cast<int>(wide.size()), result.data(), size, nullptr, nullptr);
    return result;
}

} // namespace

std::vector<WindowProcessInfo> ProcessManager::EnumerateWindows() {
    std::vector<WindowProcessInfo> results;
    EnumContext ctx{&results, ::GetForegroundWindow()};
    ::EnumWindows(EnumWindowProc, reinterpret_cast<LPARAM>(&ctx));
    return results;
}

WindowProcessInfo ProcessManager::GetForegroundApp() {
    HWND fg = ::GetForegroundWindow();
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

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        wchar_t exePath[MAX_PATH] = {};
        DWORD exePathSize = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, exePath, &exePathSize)) {
            std::wstring fullPath(exePath);
            auto pos = fullPath.find_last_of(L"\\/");
            info.exeName = (pos != std::wstring::npos) ? fullPath.substr(pos + 1) : fullPath;
        }
        CloseHandle(hProcess);
    }
    return info;
}

bool ProcessManager::LaunchProcess(const std::wstring& path, const std::wstring& args) {
    HINSTANCE result = ShellExecuteW(
        nullptr, L"open", path.c_str(),
        args.empty() ? nullptr : args.c_str(),
        nullptr, SW_SHOWNORMAL);

    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        LOG_ERROR("ShellExecute failed for: {}", WideToUtf8(path));
        return false;
    }
    return true;
}

bool ProcessManager::BringToForeground(HWND hwnd) {
    if (!IsWindow(hwnd)) return false;

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    DWORD foregroundThread = GetWindowThreadProcessId(::GetForegroundWindow(), nullptr);
    DWORD targetThread = GetWindowThreadProcessId(hwnd, nullptr);

    if (foregroundThread != targetThread) {
        AttachThreadInput(foregroundThread, targetThread, TRUE);
        SetForegroundWindow(hwnd);
        AttachThreadInput(foregroundThread, targetThread, FALSE);
    } else {
        SetForegroundWindow(hwnd);
    }

    return true;
}

bool ProcessManager::CloseWindow(HWND hwnd) {
    if (!IsWindow(hwnd)) return false;
    return PostMessageW(hwnd, WM_CLOSE, 0, 0) != 0;
}

bool ProcessManager::TerminateApp(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!hProcess) {
        LOG_ERROR("OpenProcess failed for PID {}", processId);
        return false;
    }
    BOOL result = ::TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return result != 0;
}

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
