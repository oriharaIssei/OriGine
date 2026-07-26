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

/// <summary>
/// テクスチャ管理を初期化する。
/// 基底の初期化に続けて白1x1テクスチャを読み込み、既定アセットとして確保しておく。
/// これにより、テクスチャ未指定・読み込み失敗時にもシェーダへ必ず有効なSRVを渡せる
/// （白なので乗算しても元の色を変えない、という性質を利用している）
/// </summary>
/// <param name="_capacity">事前確保するアセット数</param>
void OriGine::TextureAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);

    defaultAssetIndex_ = this->LoadAsset(kEngineResourceDirectory + "/Texture/white1x1.png");
}

/// <summary>
/// 生アセットのパスから、実際に読み込むクック済みテクスチャのパスを解決するルールを登録する
/// </summary>
void OriGine::TextureAssetManager::SetupDirectoryRules() {
    AssetManager::SetupDirectoryRules();
    // DirectoryMapper のルール追加。
    // DirectoryMapper::TryMap は登録順にルールを連鎖適用するため、この2段階の順序に意味がある。
    // 1. AssetToCookedRootRule で "resource/" ディレクトリを "cookedResource/" に置き換える
    // 2. ExtensionMappingRule で拡張子を .png/.jpg から、クック済みテクスチャの実体である .dds に置き換える
    // （テクスチャはビルド時に DDS へ変換・圧縮されて cookedResource 以下に格納される運用のため）
    directoryMapper_->AddRule(std::make_shared<AssetToCookedRootRule>());
    directoryMapper_->AddRule(std::make_shared<ExtensionMappingRule>(".png", ".dds"));
    directoryMapper_->AddRule(std::make_shared<ExtensionMappingRule>(".jpg", ".dds"));
}

/// <summary>
/// 拡張子ごとのテクスチャローダーを登録する
/// </summary>
void OriGine::TextureAssetManager::SetupLoaders() {
    // ローダーの登録。
    // ディレクトリ変換ルールにより最終的な拡張子は .dds になっている想定だが、
    // 変換対象外だった場合（.dds 以外がそのまま解決された場合）に備えて
    // defaultLoader_ には WIC（png/jpg等）ローダーを設定しておく。
    // .dds 拡張子には専用の DDS ローダー（ミップマップ生成済み前提で高速に読み込める）を割り当てる
    defaultLoader_ = std::make_unique<TextureWicLoader>();
    defaultLoader_->Initialize();
    loaderByExtension_[".dds"] = std::make_unique<TextureDdsLoader>();
    loaderByExtension_[".dds"]->Initialize();
}
