#include "TextureAssetManager.h"

/// stl
#include <memory>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
/// asset
// directoryMap
#include "asset/directoryMapper/directoryMappingRule/AssetToCookedRootRule.h"
#include "asset/directoryMapper/directoryMappingRule/ExtensionMappingRule.h"
// loader
#include "asset/loader/TextureLoader.h"

using namespace OriGine;

void OriGine::TextureAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);

    defaultAssetIndex_ = this->LoadAsset(kEngineResourceDirectory + "/Texture/white1x1.png");
}

void OriGine::TextureAssetManager::SetupDirectoryRules() {
    AssetManager::SetupDirectoryRules();
    // DirectoryMapper のルール追加
    directoryMapper_->AddRule(std::make_shared<AssetToCookedRootRule>());
    directoryMapper_->AddRule(std::make_shared<ExtensionMappingRule>(".png", ".dds"));
    directoryMapper_->AddRule(std::make_shared<ExtensionMappingRule>(".jpg", ".dds"));
}

void OriGine::TextureAssetManager::SetupLoaders() {
    // ローダーの登録
    defaultLoader_ = std::make_unique<TextureWicLoader>();
    defaultLoader_->Initialize();
    loaderByExtension_[".dds"] = std::make_unique<TextureDdsLoader>();
    loaderByExtension_[".dds"]->Initialize();
}
