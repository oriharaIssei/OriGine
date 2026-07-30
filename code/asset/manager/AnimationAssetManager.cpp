#include "AnimationAssetManager.h"

/// stl
#include <fstream>
#include <memory>

/// engine
/// asset
// directoryMap
#include "asset/directoryMapper/directoryMappingRule/AssetToCookedRootRule.h"
// loader
#include "asset/loader/AnimationLoader.h"

/// util
#include "logger/Logger.h"

using namespace OriGine;

/// <summary>
/// アニメーションアセット管理を初期化する.
/// 既定アセットは持たない（アニメーションが無い場合は再生しなければよいだけで、
/// 代替アセットを差し込む必要が無いため）。
/// </summary>
/// <param name="_capacity">事前確保するアセット数</param>
void OriGine::AnimationAssetManager::Initialize(size_t _capacity) {
    AssetManager::Initialize(_capacity);
}

/// <summary>
/// 生アセットのパスから実際に読み込むパスを解決するルールを登録する.
/// アニメーションは拡張子変換を伴わないため、クック済みルートへの差し替えのみ行う。
/// </summary>
void OriGine::AnimationAssetManager::SetupDirectoryRules() {
    AssetManager::SetupDirectoryRules();
    directoryMapper_->AddRule(std::make_shared<AssetToCookedRootRule>());
}

/// <summary>
/// 拡張子ごとのアニメーションローダーを登録する.
/// gltf / glb は assimp 経由、.anm は独自バイナリ形式のローダーが担当する。
/// </summary>
void OriGine::AnimationAssetManager::SetupLoaders() {
    defaultLoader_ = std::make_unique<GltfAnimationLoader>();
    defaultLoader_->Initialize();

    loaderByExtension_[".gltf"] = std::make_unique<GltfAnimationLoader>();
    loaderByExtension_[".gltf"]->Initialize();
    loaderByExtension_[".glb"] = std::make_unique<GltfAnimationLoader>();
    loaderByExtension_[".glb"]->Initialize();
    loaderByExtension_[".anm"] = std::make_unique<MyAnimationLoader>();
    loaderByExtension_[".anm"]->Initialize();
}

/// <summary>
/// アニメーションを独自バイナリ形式 (.anm) で保存する.
/// レイアウトは MyAnimationLoader::LoadAsset の読み込み順と対になっている.
/// </summary>
bool OriGine::AnimationSerializer::Save(const std::string& _directory, const std::string& _filename, const AnimationData& _animationData) {
    const std::string filePath = _directory + "/" + _filename + ".anm";
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) {
        LOG_ERROR("Failed to open animation file for writing: {}", filePath);
        return false;
    }

    // duration を保存
    ofs.write(reinterpret_cast<const char*>(&_animationData.duration), sizeof(_animationData.duration));

    // nodeAnimations のサイズを保存
    const size_t nodeAnimationsSize = _animationData.animationNodes_.size();
    ofs.write(reinterpret_cast<const char*>(&nodeAnimationsSize), sizeof(nodeAnimationsSize));

    // 各ノードのアニメーションデータを保存
    for (const auto& [nodeName, nodeAnimation] : _animationData.animationNodes_) {
        // ノード名の長さとノード名を保存
        const size_t nodeNameLength = nodeName.size();
        ofs.write(reinterpret_cast<const char*>(&nodeNameLength), sizeof(nodeNameLength));
        ofs.write(nodeName.c_str(), nodeNameLength);

        // scale, rotate, translate の各アニメーションカーブを保存
        auto writeCurve = [&ofs](const auto& curve) {
            const size_t size = curve.size();
            ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& keyframe : curve) {
                ofs.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));
                ofs.write(reinterpret_cast<const char*>(&keyframe.value), sizeof(keyframe.value));
            }
        };

        writeCurve(nodeAnimation.scale);
        writeCurve(nodeAnimation.rotate);
        writeCurve(nodeAnimation.translate);
    }

    return true;
}
