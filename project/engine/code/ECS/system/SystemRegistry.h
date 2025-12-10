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
/// System Registry
/// SystemRegistryはシステムの登録, 削除, 取得などを行う.
/// </summary>
class SystemRegistry final {
public:
    static SystemRegistry* GetInstance() {
        static SystemRegistry instance;
        return &instance;
    }

    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <typeparam name="SystemClass"></typeparam>
    template <IsSystem SystemClass>
    inline void RegisterSystem();

    /// <summary>
    /// システムを生成する
    /// </summary>
    /// <param name="_systemTypeName"></param>
    /// <param name="scene"></param>
    /// <returns></returns>
    std::unique_ptr<ISystem> CreateSystem(const std::string& _systemTypeName, Scene* scene);

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>> systemMaker_;

public:
    const std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>>& GetSystemMaker() const {
        return systemMaker_;
    }
    std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>>& GetSystemsRef() {
        return systemMaker_;
    }
    void ClearAll() {
        systemMaker_.clear();
    }
};

template <IsSystem SystemClass>
inline void SystemRegistry::RegisterSystem() {
    std::string systemName = nameof<SystemClass>();
    if (systemMaker_.find(systemName) != systemMaker_.end()) {
        LOG_WARN("System already registered with name: {}", systemName);
        return;
    }

    systemMaker_[systemName] = [](Scene* scene) {
        std::unique_ptr<ISystem> system = std::make_unique<SystemClass>();
        system->SetScene(scene);
        return std::move(system);
    };
}

} // namespace OriGine
