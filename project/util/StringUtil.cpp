#include "StringUtil.h"

/// api
#include <Windows.h>

/// stl
#include <algorithm>
#include <chrono>

std::wstring ConvertString(const std::string& _str) {
    if (_str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&_str[0]), static_cast<int>(_str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }

    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&_str[0]), static_cast<int>(_str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& _str) {
    if (_str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, _str.data(), static_cast<int>(_str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, _str.data(), static_cast<int>(_str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

std::string HrToString(HRESULT _hr) {
    LPWSTR errorText = nullptr;
    DWORD len        = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        _hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorText,
        0,
        nullptr);

    std::string result;
    if (len > 0 && errorText) {
        // WideChar to UTF-8
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, errorText, -1, nullptr, 0, nullptr, nullptr);
        result.resize(size_needed - 1);
        WideCharToMultiByte(CP_UTF8, 0, errorText, -1, result.data(), size_needed, nullptr, nullptr);
        LocalFree(errorText);
    } else {
        result = "Unknown error";
    }
    return result;
}

std::string NormalizeString(const std::string& _path) {
    std::string normalized = _path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized;
}

std::string TimeToString() {
    // 現在時刻を取得
    auto now        = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    // tm構造体を安全に取得
    struct tm time_info;
    localtime_s(&time_info, &time_t_now);

    // 時刻をフォーマット
    std::ostringstream oss;
    oss << std::put_time(&time_info, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}
