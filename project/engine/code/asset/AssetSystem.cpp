#include "AssetSystem.h"

/// stl
#include <format>

/// engine
// asset
#include "asset/manager/TextureAssetManager.h"

using namespace OriGine;

AssetSystem* OriGine::AssetSystem::GetInstance() {
    static AssetSystem instance;
    return &instance;
}

void OriGine::AssetSystem::Initialize() {
    // デフォルトのAssetManager郡を登録する
    auto textureManager = std::make_unique<TextureAssetManager>();
    textureManager->Initialize();

    RegisterManager<TextureAsset>(std::move(textureManager));
}

void OriGine::AssetSystem::Finalize() {
    managers_.clear();
}
