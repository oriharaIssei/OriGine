#pragma once

/// stl
#include <algorithm>
#include <filesystem>
#include <memory>
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

/// <summary>無効なアセットインデックスを表す値</summary>
constexpr size_t kInvalidAssetIndex = static_cast<size_t>(-1);

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

    // assetPathToIndexMap_ に登録したキー。
    // バリアント付きで読み込んだ場合 asset.path（ファイルパス）とは一致しないため、
    // 解放時に正しい要素を消せるようスロット側で控えておく
    std::string cacheKey;
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
    /// <returns>アセットインデックス. 読み込みに失敗した場合は既定アセットのインデックス</returns>
    size_t LoadAsset(const std::string& _assetPath) override;

    /// <summary>
    /// バリアントを指定してアセットを読み込む.
    ///
    /// 同じファイルからパラメータ違いの複数アセットを生成する型のためのオーバーロード.
    /// （シェーダをプロファイル別にコンパイルする、フォントをサイズ別に焼く、など）
    /// キャッシュキーが「パス + バリアント」になるため、同一ファイルでもバリアントが
    /// 異なれば別アセットとして管理される.
    /// </summary>
    /// <param name="_assetPath">アセットのパス</param>
    /// <param name="_variant">バリアント識別子（空文字ならバリアント無し）</param>
    /// <returns>アセットインデックス. 読み込みに失敗した場合は既定アセットのインデックス</returns>
    size_t LoadAsset(const std::string& _assetPath, const std::string& _variant);

    /// <summary>
    /// 既に生成済みのアセットを論理パスで登録する.
    /// ファイルを持たない手続き生成アセットや、エディタ上で新規作成したアセットを
    /// 読み込み済みアセットと同じインデックス管理下に置くために使う.
    /// </summary>
    /// <param name="_assetPath">登録に使う論理パス（一意であること）</param>
    /// <param name="_asset">登録するアセット</param>
    /// <returns>登録されたアセットのインデックス</returns>
    size_t RegisterAsset(const std::string& _assetPath, AssetType&& _asset);

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

    /// <summary>
    /// スロットを取得する. 未確保・解放済みの場合は nullptr.
    /// </summary>
    AssetSlot<AssetType>* TryGetSlot(size_t _assetIndex) {
        if (assets_.size() <= _assetIndex) {
            return nullptr;
        }
        auto& slot = assets_[_assetIndex];
        if (!slot || !slot->isAlive) {
            return nullptr;
        }
        return slot.get();
    }
    const AssetSlot<AssetType>* TryGetSlot(size_t _assetIndex) const {
        return const_cast<AssetManager*>(this)->TryGetSlot(_assetIndex);
    }

protected:
    // スロットは unique_ptr で保持する。
    // assets_ は再確保(push_back)で要素のアドレスが変わりうるが、
    // Model の ModelMeshData* や AnimationData* のように
    // 「アセット本体を指すポインタ」を外部が長期間保持する使い方をするため、
    // アセット本体のアドレスは安定していなければならない。
    // 併せて、GPU リソースを持つアセット(Mesh 等)を値コピーせずに済ませる意味もある。
    std::vector<std::unique_ptr<AssetSlot<AssetType>>> assets_;
    std::vector<size_t> freeIndices_;

    std::unordered_map<std::string, size_t> assetPathToIndexMap_;

    size_t defaultAssetIndex_ = kInvalidAssetIndex; // デフォルトアセット
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

    /// <summary>既定アセットのインデックスを取得する.</summary>
    size_t GetDefaultAssetIndex() const { return defaultAssetIndex_; }

    /// <summary>
    /// 読み込み済みアセットのインデックスを取得する.
    /// </summary>
    /// <returns>未読み込みの場合は kInvalidAssetIndex</returns>
    size_t GetAssetIndex(const std::string& _assetPath) const {
        auto mapIt = assetPathToIndexMap_.find(_assetPath);
        return mapIt != assetPathToIndexMap_.end() ? mapIt->second : kInvalidAssetIndex;
    }

    /// <summary>
    /// バリアント付きで読み込み済みのアセットのインデックスを取得する.
    /// </summary>
    /// <returns>未読み込みの場合は kInvalidAssetIndex</returns>
    size_t GetAssetIndex(const std::string& _assetPath, const std::string& _variant) const {
        return GetAssetIndex(MakeCacheKey(_assetPath, _variant));
    }

    /// <summary>指定インデックスのアセットが有効かどうか.</summary>
    bool IsAlive(size_t _assetIndex) const { return TryGetSlot(_assetIndex) != nullptr; }

    /// <summary>
    /// パスとバリアントからキャッシュキーを組み立てる.
    /// </summary>
    static std::string MakeCacheKey(const std::string& _assetPath, const std::string& _variant) {
        return _variant.empty() ? _assetPath : _assetPath + "#" + _variant;
    }

    /// <summary>
    /// アセットを取得する. 無効なインデックスの場合は既定アセットを返す.
    /// </summary>
    const AssetType& GetAsset(size_t _assetIndex) const {
        if (const auto* slot = TryGetSlot(_assetIndex)) {
            return slot->asset;
        }
        LOG_WARN("Asset index {} is not alive. Fallback to default asset.", _assetIndex);
        return GetDefaultAsset();
    }
    const AssetType& GetAsset(const std::string& _assetPath) const {
        const size_t index = GetAssetIndex(_assetPath);
        if (index == kInvalidAssetIndex) {
            LOG_WARN("Asset not found: {}", _assetPath);
            return GetDefaultAsset();
        }
        return GetAsset(index);
    }

    /// <summary>
    /// アセットを書き換え可能な形で取得する.
    /// </summary>
    /// <returns>無効なインデックスの場合は nullptr</returns>
    AssetType* GetMutableAsset(size_t _assetIndex) {
        auto* slot = TryGetSlot(_assetIndex);
        return slot ? &slot->asset : nullptr;
    }
    AssetType* GetMutableAsset(const std::string& _assetPath) {
        return GetMutableAsset(GetAssetIndex(_assetPath));
    }

private:
    /// <summary>
    /// 既定アセットを取得する.
    /// 既定アセットすら存在しない場合は、空アセットを返して落ちないようにする.
    /// </summary>
    const AssetType& GetDefaultAsset() const {
        if (const auto* slot = TryGetSlot(defaultAssetIndex_)) {
            return slot->asset;
        }
        static const AssetType kEmptyAsset{};
        return kEmptyAsset;
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
    assets_.clear();
    freeIndices_.clear();
    assetPathToIndexMap_.clear();

    // _capacity は「事前に確保しておく件数」であり上限ではない。
    // スロット自体は読み込み時に確保するため、ここでは領域の予約のみ行う
    assets_.reserve(_capacity);
    freeIndices_.reserve(_capacity);
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

    // ローダーは DxCommand やシェーダーコンパイラ等の実体を抱えていることがあるため、
    // 破棄する前に Finalize を呼んで解放させる
    for (auto& [extension, loader] : loaderByExtension_) {
        if (loader) {
            loader->Finalize();
        }
    }
    loaderByExtension_.clear();

    if (defaultLoader_) {
        defaultLoader_->Finalize();
    }
    defaultLoader_.reset();

    defaultAssetIndex_ = kInvalidAssetIndex;
}

template <IsAsset T>
inline size_t AssetManager<T>::LoadAsset(const std::string& _assetPath) {
    return LoadAsset(_assetPath, std::string{});
}

template <IsAsset T>
inline size_t AssetManager<T>::LoadAsset(const std::string& _assetPath, const std::string& _variant) {
    const std::string cacheKey = MakeCacheKey(_assetPath, _variant);

    // 登録されているかどうか
    auto mapIt = assetPathToIndexMap_.find(cacheKey);
    if (mapIt != assetPathToIndexMap_.end()) {
        if (auto* slot = TryGetSlot(mapIt->second)) {
            ++slot->refCount;
            return mapIt->second;
        }
        // マップには残っているがスロットが解放済み。整合性を取るためマップ側を捨てて読み込み直す
        assetPathToIndexMap_.erase(mapIt);
    }

    // アセットの読み込み
    auto mappedPath = ResolvePath(_assetPath);

    std::string extension = mappedPath.extension().string();
    // 拡張子がAssetTraitsで定義されているものか確認
    const auto& validExtensions = AssetTraits<T>::Extensions();
    if (std::find(validExtensions.begin(), validExtensions.end(), extension) == validExtensions.end()) {
        // 例外を投げるとエディタ上でのファイル指定ミスがそのままクラッシュになるため、
        // ログを出して既定アセットへフォールバックする
        LOG_ERROR("Unsupported asset extension '{}' (path: {}). Fallback to default asset.", extension, _assetPath);
        return defaultAssetIndex_;
    }

    // 拡張子に対応するローダーの取得
    IAssetLoader<AssetType>* loader = nullptr;
    auto it                         = loaderByExtension_.find(extension);
    if (it != loaderByExtension_.end()) {
        loader = it->second.get();
    } else {
        loader = defaultLoader_.get();
    }
    if (loader == nullptr) {
        LOG_ERROR("No loader available for extension '{}' (path: {}).", extension, _assetPath);
        return defaultAssetIndex_;
    }

    auto slot      = std::make_unique<AssetSlot<AssetType>>();
    slot->refCount = 1;
    slot->isAlive  = true;
    // ローダーの戻り値は move で受ける。
    // GPU リソースを保持するアセットを値コピーすると、同じリソース/マッピングを
    // 二重に所有することになり解放時に破綻するため
    slot->asset      = loader->LoadAsset(mappedPath.string(), _variant);
    slot->asset.path = _assetPath;
    slot->cacheKey   = cacheKey;

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

    // キーとインデックスのマッピングを保存
    assetPathToIndexMap_[cacheKey] = index;

    return index;
}

template <IsAsset T>
inline size_t AssetManager<T>::RegisterAsset(const std::string& _assetPath, AssetType&& _asset) {
    // 既に同じ論理パスで登録済みなら、上書きせず参照カウントだけ増やす。
    // 上書きしてしまうと、既にそのアセットを参照している側のデータが
    // 予期せず差し替わってしまうため
    auto mapIt = assetPathToIndexMap_.find(_assetPath);
    if (mapIt != assetPathToIndexMap_.end()) {
        if (auto* existing = TryGetSlot(mapIt->second)) {
            LOG_WARN("Asset '{}' is already registered. Reusing the existing one.", _assetPath);
            ++existing->refCount;
            return mapIt->second;
        }
        assetPathToIndexMap_.erase(mapIt);
    }

    auto slot        = std::make_unique<AssetSlot<AssetType>>();
    slot->refCount   = 1;
    slot->isAlive    = true;
    slot->asset      = std::move(_asset);
    slot->asset.path = _assetPath;
    slot->cacheKey   = _assetPath;

    size_t index;
    if (!freeIndices_.empty()) {
        index = freeIndices_.back();
        freeIndices_.pop_back();
        assets_[index] = std::move(slot);
    } else {
        index = assets_.size();
        assets_.push_back(std::move(slot));
    }

    assetPathToIndexMap_[_assetPath] = index;

    return index;
}

template <IsAsset T>
inline void AssetManager<T>::ReleaseAsset(size_t _assetIndex) {
    auto* slot = TryGetSlot(_assetIndex);
    if (slot == nullptr) {
        LOG_ERROR("Asset at index {} is not alive.", _assetIndex);
        return;
    }
    if (slot->refCount == 0) {
        LOG_ERROR("Asset at index {} has no references to release.", _assetIndex);
        return;
    }

    --slot->refCount;
    if (slot->refCount != 0) {
        return;
    }

    // 参照が尽きたのでスロットごと破棄する(GPU / CPU リソースの解放)
    assetPathToIndexMap_.erase(slot->cacheKey);
    assets_[_assetIndex].reset();
    freeIndices_.push_back(_assetIndex);
}

template <IsAsset T>
inline void AssetManager<T>::ReleaseAsset(const std::string& _assetPath) {
    auto mapIt = assetPathToIndexMap_.find(_assetPath);
    if (mapIt == assetPathToIndexMap_.end()) {
        LOG_ERROR("Asset not found: {}", _assetPath);
        return;
    }
    // マップの削除は ReleaseAsset(index) 側が参照カウント 0 を確認してから行う。
    // ここで無条件に erase すると、まだ参照が残っているアセットが
    // 再読み込み時に別スロットへ二重ロードされてしまう
    ReleaseAsset(mapIt->second);
}

} // namespace OriGine
