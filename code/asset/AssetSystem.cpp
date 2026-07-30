#include "AssetSystem.h"

/// stl
#include <format>

/// engine
// asset
#include "asset/manager/AnimationAssetManager.h"
#include "asset/manager/ModelAssetManager.h"
#include "asset/manager/ShaderAssetManager.h"
#include "asset/manager/SoundAssetManager.h"
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
    //
    // アセットの型(TextureAsset 等)をキーとして IAssetManager を登録する。
    // 以後、型ごとの AssetManager は GetManager<T>() 等でこのレジストリから取得される

    // テクスチャは他のアセット(モデルのマテリアル等)から参照されるため、必ず最初に登録する
    auto textureManager = std::make_unique<TextureAssetManager>();
    textureManager->Initialize();
    RegisterManager<TextureAsset>(std::move(textureManager));

    // モデルの読み込みはマテリアルのテクスチャ読み込みを伴うため、テクスチャの後に登録する
    auto modelManager = std::make_unique<ModelAssetManager>();
    modelManager->Initialize();
    RegisterManager<ModelAsset>(std::move(modelManager));

    auto animationManager = std::make_unique<AnimationAssetManager>();
    animationManager->Initialize();
    RegisterManager<AnimationAsset>(std::move(animationManager));

    auto soundManager = std::make_unique<SoundAssetManager>();
    soundManager->Initialize();
    RegisterManager<SoundAsset>(std::move(soundManager));

    auto shaderManager = std::make_unique<ShaderAssetManager>();
    shaderManager->Initialize();
    RegisterManager<ShaderAsset>(std::move(shaderManager));
}

/// <summary>
/// 登録済みのAssetManagerをすべて破棄する。
/// managers_ は unique_ptr で保持しているため、clear() でそれぞれのデストラクタが走り、
/// 各マネージャが抱えるGPUリソース(テクスチャ等)もまとめて解放される
/// </summary>
void OriGine::AssetSystem::Finalize() {
    managers_.clear();
}
