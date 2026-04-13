#pragma once

/// stl
#include <filesystem>

namespace OriGine {

/// <summary>
/// Directory の変換ルールを表すインターフェース
/// </summary>
class IDirectoryMappingRule {
public:
    virtual ~IDirectoryMappingRule() = default;

    /// <summary>
    /// Directory を変換する
    /// </summary>
    /// <param name="_directory"></param>
    /// <returns></returns>
    virtual std::filesystem::path Apply(const std::filesystem::path& _directory) const = 0;
};

} // namespace OriGine
