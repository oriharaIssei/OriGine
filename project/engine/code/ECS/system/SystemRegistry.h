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
    /// システムを登録する(RenderSystem,PostRenderSystem以外)
    /// </summary>
    /// <typeparam name="SystemClass"></typeparam>
    template <IsSystem SystemClass>
    inline void registerSystem();

    /// <summary>
    /// レンダリングシステムを登録する
    /// </summary>
    template <IsRenderSystem RenderSystemClass>
    inline void registerRenderSystem();

    template <IsPostRenderSystem PostRenderSystemClass>
    inline void registerPostRenderSystem();

    /// <summary>
    /// システムを生成する
    /// </summary>
    /// <param name="_systemTypeName"></param>
    /// <param name="scene"></param>
    /// <returns></returns>
    std::unique_ptr<ISystem> CreateSystem(const std::string& _systemTypeName, Scene* scene);

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<ISystem>(Scene*)>> systemMaker_;
    std::unordered_map<std::string, std::function<std::unique_ptr<BaseRenderSystem>(Scene*)>> renderSystemMaker_;
    std::unordered_map<std::string, std::function<std::unique_ptr<BasePostRenderingSystem>(Scene*)>> postRenderSystemMaker_;

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

template <IsRenderSystem RenderSystemClass>
inline void SystemRegistry::registerRenderSystem() {
    std::string systemName = nameof<RenderSystemClass>();
    if (systemMaker_.find(systemName) != systemMaker_.end()) {
        LOG_WARN("SystemRegistry: RenderSystem already registered with name: {}", systemName);
        return;
    }
    renderSystemMaker_[systemName] = [](Scene* scene) {
        std::unique_ptr<BaseRenderSystem> system = std::make_unique<RenderSystemClass>();
        system->setScene(scene);
        return std::move(system);
    };
}

template <IsPostRenderSystem PostRenderSystemClass>
inline void SystemRegistry::registerPostRenderSystem() {
    std::string systemName = nameof<PostRenderSystemClass>();
    if (systemMaker_.find(systemName) != systemMaker_.end()) {
        LOG_WARN("SystemRegistry: PostRenderSystem already registered with name: {}", systemName);
        return;
    }
    postRenderSystemMaker_[systemName] = [](Scene* scene) {
        std::unique_ptr<BasePostRenderingSystem> system = std::make_unique<PostRenderSystemClass>();
        system->setScene(scene);
        return std::move(system);
    };
}
