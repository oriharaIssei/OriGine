#pragma once

/// stl
#include <memory>
#include <unordered_map>

/// ECS
#include "ECS/system/ISystem.h"

/// external
#include "logger/Logger.h"

namespace OriGine {

/// <summary>
/// 全てのシステムを管理・生成するためのレジストリクラス
/// </summary>
class SystemRegistry final {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    /// <returns>SystemRegistryのインスタンス</returns>
    static SystemRegistry* GetInstance() {
        static SystemRegistry instance;
        return &instance;
    }

    /// <summary>
    /// システムをレジストリに登録する（生成関数を登録）
    /// </summary>
    /// <typeparam name="SystemClass">システムクラスの型</typeparam>
    template <IsSystem SystemClass>
    inline void RegisterSystem();

    /// <summary>
    /// 登録済みのシステム名からインスタンスを生成する
    /// </summary>
    /// <param name="_systemTypeName">システムの型名</param>
    /// <param name="scene">所属させるシーンのポインタ</param>
    /// <returns>生成されたシステムの共有ポインタ (未登録時はnullptr)</returns>
    std::shared_ptr<ISystem> CreateSystem(const std::string& _systemTypeName, Scene* scene);

private:
    /// <summary>
    /// システム名と生成（メイカー）関数のマップ
    /// </summary>
    std::unordered_map<std::string, std::function<std::shared_ptr<ISystem>(Scene*)>> systemMaker_;

public:
    /// <summary>
    /// システムメイカーのマップを取得する
    /// </summary>
    /// <returns>システム名と生成関数のマップへの参照</returns>
    const std::unordered_map<std::string, std::function<std::shared_ptr<ISystem>(Scene*)>>& GetSystemMaker() const {
        return systemMaker_;
    }

    /// <summary>
    /// システムメイカーのマップを取得する
    /// </summary>
    /// <returns>システム名と生成関数のマップへの参照</returns>
    std::unordered_map<std::string, std::function<std::shared_ptr<ISystem>(Scene*)>>& GetSystemsRef() {
        return systemMaker_;
    }

    /// <summary>
    /// 全てのシステム登録を解除する
    /// </summary>
    void ClearAll() {
        systemMaker_.clear();
    }
};

/// <summary>
/// システムをレジストリに登録する
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
template <IsSystem SystemClass>
inline void SystemRegistry::RegisterSystem() {
    std::string systemName = nameof<SystemClass>();
    if (systemMaker_.find(systemName) != systemMaker_.end()) {
        LOG_WARN("System already registered with name: {}", systemName);
        return;
    }

    systemMaker_[systemName] = [](Scene* scene) {
        std::shared_ptr<ISystem> system = std::make_shared<SystemClass>();
        system->SetScene(scene);
        return std::move(system);
    };
}

} // namespace OriGine
