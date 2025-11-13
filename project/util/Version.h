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

    std::string toString() const;
    void fromString(const std::string& str);
};
