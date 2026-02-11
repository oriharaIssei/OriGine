#pragma once

/// stl
#include <filesystem>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
// asset
#include "asset/Asset.h"
#include "asset/directoryMapper/DirectoryMapper.h"
#include "asset/loader/IAssetLoader.h"
// logger
#include "logger/Logger.h"

namespace OriGine {
class IAssetManager {
public:
    virtual ~IAssetManager() = default;

    virtual void Initialize(size_t _capacity) = 0;
    virtual void Finalize()                   = 0;

    virtual size_t LoadAsset(const std::string& _assetPath) = 0;

    virtual void ReleaseAsset(size_t _assetIndex)            = 0;
    virtual void ReleaseAsset(const std::string& _assetPath) = 0;

    /// <summary>
    /// 論理パスを変換ルールに基づいて解決する
    /// </summary>
    /// <param name="logicalPath"></param>
    /// <returns></returns>
    std::filesystem::path ResolvePath(const std::string& logicalPath) const;

protected:
    std::unique_ptr<DirectoryMapper> directoryMapper_;
};

/// <summary>
/// アセットスロット
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
struct AssetSlot {
    T asset;
    size_t refCount = 0;
    bool isAlive    = false;
};

/// <summary>
/// Asset の管理を担当するクラス.
/// 特殊化して使用する.
/// </summary>
/// <typeparam name="T"></typeparam>
template <IsAsset T>
class AssetManager
    : public IAssetManager {
    using AssetType = typename AssetTraits<T>::type;

public:
    AssetManager()          = default;
    virtual ~AssetManager() = default;

    virtual void Initialize(size_t _capacity);
    virtual void Finalize();

    /// <summary>
    /// アセットを読み込む
    /// </summary>
    /// <param name="_assetPath"></param>
    /// <returns></returns>
    size_t LoadAsset(const std::string& _assetPath) override;

    /// <summary>
    /// 指定されたアセットをアンロードする.
    /// </summary>
    /// <param name="_assetIndex"></param>
    void ReleaseAsset(size_t _assetIndex) override;
    /// <summary>
    /// 指定されたアセットをアンロードする.
    /// </summary>
    /// <param name="_assetPath"></param>
    void ReleaseAsset(const std::string& _assetPath) override;

protected:
    /// <summary>
    /// ストレージの初期化
    /// </summary>
    /// <param name="_capacity"></param>
    virtual void InitializeStorage(size_t _capacity);
    /// <summary>
    /// ディレクトリ変換ルールの設定
    /// directoryMapper の初期化もここで行う
    /// </summary>
    virtual void SetupDirectoryRules();
    /// <summary>
    /// ローダーの設定
    /// </summary>
    virtual void SetupLoaders() {}

protected:
    std::vector<AssetSlot<AssetType>> assets_;
    std::vector<size_t> freeIndices_;

    std::unordered_map<std::string, size_t> assetPathToIndexMap_;

    size_t defaultAssetIndex_ = static_cast<size_t>(0); // デフォルトアセット
    std::unique_ptr<IAssetLoader<AssetType>> defaultLoader_; // デフォルトローダー
    std::unordered_map<std::string, std::unique_ptr<IAssetLoader<AssetType>>> loaderByExtension_; // 拡張子ごとのローダーマップ
public:
    IAssetLoader<AssetType>* GetDefaultLoader() const {
        return defaultLoader_.get();
    }
    IAssetLoader<AssetType>* GetLoaderForExtension(const std::string& _extension) const {
        auto it = loaderByExtension_.find(_extension);
        if (it != loaderByExtension_.end()) {
            return it->second.get();
        }
        return defaultLoader_.get();
    }

    const AssetType& GetAsset(size_t _assetIndex) const {
        if (assets_.size() <= _assetIndex || !assets_.at(_assetIndex).isAlive) {
            LOG_WARN("Asset index {} is out of range.", _assetIndex);
            return assets_.at(defaultAssetIndex_).asset;
        }

        return assets_.at(_assetIndex).asset;
    }
    const AssetType& GetAsset(const std::string& _assetPath) const {
        auto mapIt = assetPathToIndexMap_.find(_assetPath);
        if (mapIt == assetPathToIndexMap_.end()) {
            LOG_WARN("Asset not found: {}", _assetPath);
            return assets_.at(defaultAssetIndex_).asset;
        }
        return GetAsset(mapIt->second);
    }
};

template <IsAsset T>
inline void AssetManager<T>::Initialize(size_t _capacity) {
    InitializeStorage(_capacity);
    SetupDirectoryRules();
    SetupLoaders();
}

template <IsAsset T>
inline void AssetManager<T>::InitializeStorage(size_t _capacity) {
    // 既存データのクリア
    if (!assets_.empty()) {
        assets_.clear();
    }
    if (!freeIndices_.empty()) {
        freeIndices_.clear();
    }
    // データ構造の初期化
    assets_.resize(_capacity);
    freeIndices_.resize(_capacity);
    std::iota(freeIndices_.begin(), freeIndices_.end(), 0);
}

template <IsAsset T>
inline void AssetManager<T>::SetupDirectoryRules() {
    directoryMapper_ = std::make_unique<DirectoryMapper>();
    directoryMapper_->Initialize();
}

template <IsAsset T>
inline void AssetManager<T>::Finalize() {
    assets_.clear();
    freeIndices_.clear();
    assetPathToIndexMap_.clear();
    loaderByExtension_.clear();
    defaultLoader_.reset();
}

template <IsAsset T>
inline size_t AssetManager<T>::LoadAsset(const std::string& _assetPath) {
    // 登録されているかどうか
    auto mapIt = assetPathToIndexMap_.find(_assetPath);
    if (mapIt != assetPathToIndexMap_.end()) {
        auto& slot = assets_[mapIt->second];
        ++slot.refCount;
        return mapIt->second;
    }

    // アセットの読み込み
    auto mappedPath = ResolvePath(_assetPath);

    std::string extension = mappedPath.extension().string();
    // 拡張子がAssetTraitsで定義されているものか確認
    const auto& validExtensions = AssetTraits<T>::Extensions();
    if (std::find(validExtensions.begin(), validExtensions.end(), extension) == validExtensions.end()) {
        throw std::runtime_error("Unsupported asset extension: " + extension);
    }


    // 拡張子に対応するローダーの取得
    IAssetLoader<T>* loader = nullptr;
    auto it                 = loaderByExtension_.find(extension);
    if (it != loaderByExtension_.end()) {
        loader = it->second.get();
    } else {
        loader = defaultLoader_.get();
    }


    AssetType asset = loader->LoadAsset(mappedPath.string());
    asset.path      = _assetPath;

    AssetSlot<AssetType> slot;
    slot.asset    = asset;
    slot.refCount = 1;
    slot.isAlive  = true;

    // アセットの格納
    size_t index;
    if (!freeIndices_.empty()) {
        index = freeIndices_.back();
        freeIndices_.pop_back();
        assets_[index] = std::move(slot);
    } else {
        index = assets_.size();
        assets_.push_back(std::move(slot));
    }

    return index;
}

template <IsAsset T>
inline void AssetManager<T>::ReleaseAsset(size_t _assetIndex) {
    auto& slot = assets_.at(_assetIndex);
    if (slot.refCount <= 0) {
        LOG_ERROR("Asset at index {} has no references to release.", _assetIndex);
        return;
    }

    slot.refCount--;
    if (slot.refCount == 0) {
        slot.asset   = AssetType{}; // GPU / CPU resource 解放
        slot.isAlive = false;
        freeIndices_.push_back(_assetIndex);
    }
}

template <IsAsset T>
inline void AssetManager<T>::ReleaseAsset(const std::string& _assetPath) {
    auto mapIt = assetPathToIndexMap_.find(_assetPath);
    if (mapIt == assetPathToIndexMap_.end()) {
        LOG_ERROR("Asset not found: {}", _assetPath);
        return;
    }
    ReleaseAsset(mapIt->second);
    assetPathToIndexMap_.erase(mapIt);
}

} // namespace OriGine
