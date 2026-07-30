#include "ShaderAssetManager.h"

/// stl
#include <memory>

/// engine
/// asset
// loader
#include "asset/loader/ShaderLoader.h"

using namespace OriGine;

/// <summary>
/// シェーダーアセット管理を初期化する.
/// 既定アセットは持たない（シェーダーが無い状態で代替バイナリを差し込んでも
/// PSO 生成が通らないため、読み込み失敗はログで気付ける方がよい）。
/// </summary>
/// <param name="_capacity">事前確保するアセット数</param>
void OriGine::ShaderAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);
}

/// <summary>
/// シェーダーローダーを登録する.
/// HLSL はクック対象外（実行時に DXC でコンパイルする）ため、
/// SetupDirectoryRules は基底のまま（変換ルール無し）でよい。
/// </summary>
void OriGine::ShaderAssetManager::SetupLoaders() {
    defaultLoader_ = std::make_unique<ShaderLoader>();
    defaultLoader_->Initialize();
    loaderByExtension_[".hlsl"] = std::make_unique<ShaderLoader>();
    loaderByExtension_[".hlsl"]->Initialize();
}
