#include "AssetToCookedRootRule.h"

using namespace OriGine;

AssetToCookedRootRule::AssetToCookedRootRule() {}
AssetToCookedRootRule::~AssetToCookedRootRule() {}

std::filesystem::path AssetToCookedRootRule::Apply(const std::filesystem::path& _directory) const {
    // ./ を除去
    std::filesystem::path normalized = _directory.lexically_normal();

    auto it = normalized.begin();
    if (it == normalized.end()) {
        return _directory;
    }

    const std::filesystem::path first = *it; // engine / application
    ++it;
    if (it == normalized.end()) {
        return _directory;
    }

    if (it->string() != assetRootDirectoryName_) {
        return _directory;
    }

    std::filesystem::path result;
    result /= first;
    result /= cookedRootDirectoryName_;

    ++it;
    for (; it != normalized.end(); ++it) {
        result /= *it;
    }

    return result;
}
