#pragma once

#include "IDirectoryMappingRule.h"

/// stl
#include <string>

namespace OriGine {

/// <summary>
/// Asset ディレクトリを CookedRoot ディレクトリに変換するルール
/// </summary>
class AssetToCookedRootRule
    : public IDirectoryMappingRule {
public:
    AssetToCookedRootRule();
    ~AssetToCookedRootRule() override;

    std::filesystem::path Apply(const std::filesystem::path& _directory) const override;

private:
    std::string assetRootDirectoryName_  = "resource";
    std::string cookedRootDirectoryName_ = "cookedResource";
};
}
