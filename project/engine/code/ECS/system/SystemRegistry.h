#pragma once

/// stl
#include <memory>
#include <unordered_map>

/// ECS
#include "ECS/system/ISystem.h"
#include "ECS/system/postRender/base/BasePostRenderingSystem.h"
#include "ECS/system/render/base/BaseRenderSystem.h"

/// external
#include "logger/Logger.h"

/// <summary>
/// System Registry
/// SystemRegistryはシステムの登録, 削除, 取得などを行う.
/// </summary>
class SystemRegistry final {
public:
    static SystemRegistry* getInstance() {
        static SystemRegistry instance;
        return &instance;
    }

    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <typeparam name="SystemClass"></typeparam>
    template <IsSystem SystemClass>
    inline void registerSystem();

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
    const std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>>& getSystemMaker() const {
        return systemMaker_;
    }
    std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>>& getSystemsRef() {
        return systemMaker_;
    }
    void clearAll() {
        systemMaker_.clear();
    }
};

template <IsSystem SystemClass>
inline void SystemRegistry::registerSystem() {
    std::string systemName = nameof<SystemClass>();
    if (systemMaker_.find(systemName) != systemMaker_.end()) {
        LOG_WARN("SystemRegistry: System already registered with name: {}", systemName);
        return;
    }

    systemMaker_[systemName] = [](Scene* scene) {
        std::unique_ptr<ISystem> system = std::make_unique<SystemClass>();
        system->setScene(scene);
        return std::move(system);
    };
}
