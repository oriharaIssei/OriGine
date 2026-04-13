#pragma once

/// stl
#include <string>

/// math
#include <cstdint>

/// <summary>
/// バージョン情報を表す構造体
/// </summary>
struct Version {
    uint32_t major = 1;
    uint32_t minor = 0;
    uint32_t patch = 0;

    /// <summary>
    /// 文字列へ変換 ("major.minor.patch" 形式)
    /// </summary>
    /// <returns>バージョン文字列</returns>
    std::string toString() const;
    /// <summary>
    /// 文字列からパース
    /// </summary>
    /// <param name="str">バージョン文字列</param>
    void fromString(const std::string& str);
};
