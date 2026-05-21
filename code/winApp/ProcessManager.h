#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace OriGine {

struct WindowProcessInfo {
    DWORD processId = 0;
    HWND hwnd = nullptr;
    std::wstring exeName;
    std::wstring windowTitle;
    bool isForeground = false;
};

class ProcessManager {
public:
    static std::vector<WindowProcessInfo> EnumerateWindows();
    static WindowProcessInfo GetForegroundApp();

    static bool LaunchProcess(const std::wstring& path, const std::wstring& args = L"");
    static bool BringToForeground(HWND hwnd);
    static bool CloseWindow(HWND hwnd);
    static bool TerminateApp(DWORD processId);

    static std::string FormatAsText(const std::vector<WindowProcessInfo>& windows);
};

} // namespace OriGine
