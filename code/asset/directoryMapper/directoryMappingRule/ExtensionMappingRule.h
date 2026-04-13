#pragma once

#include "IDirectoryMappingRule.h"

namespace OriGine {

/// <summary>
/// 拡張子による Directory 変換ルール
/// </summary>
class ExtensionMappingRule
    : public IDirectoryMappingRule {
public:
    ExtensionMappingRule(const std::string& _fromExtension, const std::string& _toExtension);
    ~ExtensionMappingRule() override = default;

    std::filesystem::path Apply(const std::filesystem::path& _directory) const override;

private:
    std::string fromExtension_;
    std::string toExtension_;
};

} // namespace OriGine
