#include "AssetManager.h"

/// util
#include <logger/Logger.h>

std::filesystem::path OriGine::IAssetManager::ResolvePath(const std::string& _logicalPath) const { // アセットの読み込み
    auto mappedPath = directoryMapper_->TryMap(_logicalPath);

    if (!std::filesystem::exists(mappedPath)) {
        LOG_WARN("Cooked asset not found. Fallback to source: {}", mappedPath.string());
        mappedPath = _logicalPath;
    }
    return mappedPath;
}
