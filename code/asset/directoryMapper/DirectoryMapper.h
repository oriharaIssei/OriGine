#pragma once

/// stl
#include <filesystem>
#include <memory>
#include <vector>

/// engine
// asset
#include "asset/directoryMapper/directoryMappingRule/IDirectoryMappingRule.h"

namespace OriGine {

/// <summary>
/// Directory をルールによって変換するクラス
/// </summary>
class DirectoryMapper {
public:
    DirectoryMapper();
    ~DirectoryMapper();

    void Initialize();
    void Finalize();

    /// <summary>
    /// 変換ルールを追加する
    /// </summary>
    /// <param name="_rule"></param>
    void AddRule(const std::shared_ptr<IDirectoryMappingRule>& _rule);

    /// <summary>
    /// Directory を変換する
    /// </summary>
    /// <param name="_directory"></param>
    /// <returns></returns>
    std::filesystem::path TryMap(const std::filesystem::path& _directory) const;

private:
    std::vector<std::shared_ptr<IDirectoryMappingRule>> rules_;
};

} // namespace OriGine
