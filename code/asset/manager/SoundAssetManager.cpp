#include "SoundAssetManager.h"

/// stl
#include <memory>

/// engine
/// asset
// directoryMap
#include "asset/directoryMapper/directoryMappingRule/AssetToCookedRootRule.h"
// loader
#include "asset/loader/WaveLoader.h"

using namespace OriGine;

/// <summary>
/// 音声アセット管理を初期化する.
/// テクスチャと違い「必ず有効な既定アセットを返す」必要がある場面が無い
/// （音が鳴らないだけで描画は破綻しない）ため、既定アセットは読み込まない。
/// 未読み込みのインデックスを引いた場合は空の SoundAsset が返る。
/// </summary>
/// <param name="_capacity">事前確保するアセット数</param>
void OriGine::SoundAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);
}

/// <summary>
/// 生アセットのパスから、実際に読み込む音声ファイルのパスを解決するルールを登録する.
/// 音声はクック対象外の運用でも、cookedResource 側に配置された場合はそちらを優先できるよう
/// AssetToCookedRootRule のみを登録する（存在しなければ ResolvePath が生アセットへフォールバックする）。
/// </summary>
void OriGine::SoundAssetManager::SetupDirectoryRules() {
    AssetManager::SetupDirectoryRules();
    directoryMapper_->AddRule(std::make_shared<AssetToCookedRootRule>());
}

/// <summary>
/// WAVE ローダーを登録する.
/// </summary>
void OriGine::SoundAssetManager::SetupLoaders() {
    defaultLoader_ = std::make_unique<WaveLoader>();
    defaultLoader_->Initialize();
    loaderByExtension_[".wav"] = std::make_unique<WaveLoader>();
    loaderByExtension_[".wav"]->Initialize();
}
