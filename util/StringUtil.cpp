#include "StringUtil.h"

/// api
#include <Windows.h>

/// stl
#include <algorithm>
#include <chrono>

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

std::string NormalizeString(const std::string& path) {
    std::string normalized = path;
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

std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    return result;
}

std::string Trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); });
    auto end   = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

std::string TrimAfterNewline(const std::string& str, bool includeNewline) {
    size_t pos = str.find_first_of("\r\n");
    if (pos == std::string::npos) {
        return str;
    }
    if (includeNewline) {
        // 改行文字の直後まで含める (\r\n の場合は 2 文字分)
        size_t end = pos + 1;
        if (str[pos] == '\r' && end < str.size() && str[end] == '\n') {
            ++end;
        }
        return str.substr(0, end);
    }
    return str.substr(0, pos);
}
