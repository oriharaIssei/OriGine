#include "ConvertString.h"

/// api
#include <Windows.h>

std::wstring ConvertString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }

    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

std::string HrToString(HRESULT hr) {
    LPWSTR errorText = nullptr;
    DWORD len        = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
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
