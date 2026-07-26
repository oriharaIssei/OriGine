#include "AssetSystem.h"

/// stl
#include <format>

/// engine
// asset
#include "asset/manager/TextureAssetManager.h"

using namespace OriGine;

/// <summary>
/// シングルトンインスタンスを取得する
/// </summary>
/// <returns>唯一のAssetSystemインスタンス</returns>
AssetSystem* OriGine::AssetSystem::GetInstance() {
    static AssetSystem instance;
    return &instance;
}

/// <summary>
/// 型ごとのAssetManagerを生成・登録し、アセット管理を利用可能な状態にする
/// </summary>
void OriGine::AssetSystem::Initialize() {
    // デフォルトのAssetManager郡を登録する
    // 各 AssetManager は Initialize() の中でデフォルトアセット(white1x1.png 等)の読み込みを行うため、
    // RegisterManager に渡す前に必ず Initialize() を済ませておく
    auto textureManager = std::make_unique<TextureAssetManager>();
    textureManager->Initialize();

    // アセットの型(TextureAsset)をキーとして IAssetManager を登録する。
    // 以後、型ごとの AssetManager は GetManager<T>() 等でこのレジストリから取得される
    RegisterManager<TextureAsset>(std::move(textureManager));
}

/// <summary>
/// 登録済みのAssetManagerをすべて破棄する。
/// managers_ は unique_ptr で保持しているため、clear() でそれぞれのデストラクタが走り、
/// 各マネージャが抱えるGPUリソース(テクスチャ等)もまとめて解放される
/// </summary>
void OriGine::AssetSystem::Finalize() {
    managers_.clear();
}
