#pragma once

/// stl
#include <memory>
#include <typeindex>
#include <unordered_map>

/// engine
// asset
#include "Asset.h"
#include "asset/manager/AssetManager.h"

/// util
#include "logger/Logger.h"
#include "util/nameof.h"

namespace OriGine {

/// <summary>
/// アセットシステム
/// </summary>
class AssetSystem {
public:
    static AssetSystem* GetInstance();

public:
    void Initialize();
    void Finalize();

    /// <summary>
    /// アセットマネージャーを登録する.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="_manager"></param>
    /// <returns></returns>
    template <IsAsset T>
    bool RegisterManager(std::unique_ptr<AssetManager<T>> _manager);
    /// <summary>
    /// アセットマネージャーの登録を解除する.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template <IsAsset T>
    bool UnregisterManager();

    /// <summary>
    /// アセットを読み込む
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="_assetPath"></param>
    /// <returns></returns>
    template <IsAsset T>
    size_t LoadAsset(const std::string& _assetPath);

    /// <summary>
    /// 指定されたアセットをアンロードする.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="_assetIndex"></param>
    template <IsAsset T>
    void ReleaseAsset(size_t _assetIndex);
    /// <summary>
    /// 指定されたアセットをアンロードする.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="_assetPath"></param>
    template <IsAsset T>
    void ReleaseAsset(const std::string& _assetPath);

private:
    AssetSystem()                              = default;
    ~AssetSystem()                             = default;
    AssetSystem& operator=(const AssetSystem&) = delete;
    AssetSystem(const AssetSystem&)            = delete;

private:
    std::unordered_map<std::type_index, std::unique_ptr<IAssetManager>> managers_;

public:
    template <IsAsset T>
    AssetManager<T>* GetManager();
};

template <IsAsset T>
inline bool AssetSystem::RegisterManager(std::unique_ptr<AssetManager<T>> _manager) {
    std::type_index typeIndex(typeid(T));
    if (managers_.find(typeIndex) != managers_.end()) {
        LOG_WARN("Asset manager for type {} is already registered.", nameof<T>());
        return false;
    }
    managers_[typeIndex] = std::move(_manager);
    return true;
}

template <IsAsset T>
inline bool AssetSystem::UnregisterManager() {
    std::type_index typeIndex(typeid(T));
    auto it = managers_.find(typeIndex);
    if (it != managers_.end()) {
        managers_.erase(it);
        return true;
    }
    LOG_WARN("Asset manager for type {} is not registered.", nameof<T>());
    return false;
}

template <IsAsset T>
inline size_t AssetSystem::LoadAsset(const std::string& _assetPath) {
    AssetManager<T>* manager = GetManager<T>();
    if (!manager) {
        LOG_ERROR("Asset manager for type {} is not registered.", nameof<T>());
        return static_cast<size_t>(-1);
    }
    return manager->LoadAsset(_assetPath);
}

template <IsAsset T>
inline void AssetSystem::ReleaseAsset(size_t _assetIndex) {
    AssetManager<T>* manager = GetManager<T>();
    if (!manager) {
        LOG_ERROR("Asset manager for type {} is not registered.", nameof<T>());
        return;
    }
    manager->ReleaseAsset(_assetIndex);
}

template <IsAsset T>
inline void AssetSystem::ReleaseAsset(const std::string& _assetPath) {
    AssetManager<T>* manager = GetManager<T>();
    if (!manager) {
        LOG_ERROR("Asset manager for type {} is not registered.", nameof<T>());
        return;
    }
    manager->ReleaseAsset(_assetPath);
}

template <IsAsset T>
inline AssetManager<T>* AssetSystem::GetManager() {
    std::type_index typeIndex(typeid(T));
    auto it = managers_.find(typeIndex);
    if (it != managers_.end()) {
        return static_cast<AssetManager<T>*>(it->second.get());
    }
    return nullptr;
}

}
