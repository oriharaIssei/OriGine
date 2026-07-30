#include "ModelAssetManager.h"

/// stl
#include <memory>

/// engine
/// asset
// directoryMap
#include "asset/directoryMapper/directoryMappingRule/AssetToCookedRootRule.h"
// loader
#include "asset/loader/ModelLoader.h"

using namespace OriGine;

/// <summary>
/// モデルアセット管理を初期化する.
/// 既定アセットは持たない（モデルが読めない場合は何も描画しないのが正しく、
/// 代替モデルを差し込むと問題が見えにくくなるため）。
/// </summary>
/// <param name="_capacity">事前確保するアセット数</param>
void OriGine::ModelAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);
}

/// <summary>
/// 生アセットのパスから実際に読み込むパスを解決するルールを登録する.
/// モデルは拡張子変換を伴わないため、クック済みルートへの差し替えのみ行う
/// （cookedResource 側に無ければ ResolvePath が生アセットへフォールバックする）。
/// </summary>
void OriGine::ModelAssetManager::SetupDirectoryRules() {
    AssetManager::SetupDirectoryRules();
    directoryMapper_->AddRule(std::make_shared<AssetToCookedRootRule>());
}

/// <summary>
/// モデルローダーを登録する.
/// 対応形式はいずれも assimp が解釈するため、拡張子ごとに分けず既定ローダーで受ける。
/// </summary>
void OriGine::ModelAssetManager::SetupLoaders() {
    defaultLoader_ = std::make_unique<ModelLoader>();
    defaultLoader_->Initialize();
}
