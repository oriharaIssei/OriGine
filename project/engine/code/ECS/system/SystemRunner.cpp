#include "SystemRunner.h"

/// ECS
// system
#include "SystemRegistry.h"

/// external
#include "logger/Logger.h"

namespace OriGine {

SystemRunner::SystemRunner(Scene* _scene) : scene_(_scene) {}
SystemRunner::~SystemRunner() {}

void SystemRunner::InitializeAllCategory() {
    for (auto& [name, system] : systems_) {
        system->Initialize();
    }
}

void SystemRunner::InitializeCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto& [systemName, system] : systems_) {
        if (system) {
            system->Initialize();
        }
    }
}

void SystemRunner::InitializeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto system : systemByCategory) {
            if (system) {
                system->Initialize();
            }
        }
    }
}

void SystemRunner::InitializeActiveCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto system : activeSystems_[static_cast<size_t>(_category)]) {
        if (system) {
            system->Initialize();
        }
    }
}

void SystemRunner::FinalizeAllCategory() {
    for (auto& [systemName, system] : systems_) {
        if (system) {
            system->Finalize();
        }
    }
}

void SystemRunner::FinalizeCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto& [systemName, system] : systems_) {
        if (system) {
            system->Finalize();
        }
    }
}

void SystemRunner::FinalizeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto system : systemByCategory) {
            if (system) {
                system->Finalize();
            }
        }
    }
}

void SystemRunner::FinalizeActiveCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }

    auto categoryIndex = static_cast<size_t>(_category);
    if (categoryIndex >= activeSystems_.size()) {
        LOG_ERROR("SystemRunner: Invalid SystemCategory index: {}", categoryIndex);
        return;
    }
    for (auto system : activeSystems_[categoryIndex]) {
        if (system) {
            system->Finalize();
        }
    }
}

void SystemRunner::AllUnregisterSystem(bool _isFinalize) {
    for (auto& [systemName, system] : systems_) {
        if (!system) {
            continue;
        }
        system->SetScene(nullptr);
        system->SetIsActive(false);
        if (_isFinalize) {
            system->Finalize();
        }
    }
    for (size_t i = 0; i < static_cast<size_t>(SystemCategory::Count); ++i) {
        activeSystems_[i].clear();
    }
    systems_.clear();
}

void SystemRunner::UpdateCategory(SystemCategory _category) {
    if (!categoryActivity[static_cast<size_t>(_category)]) {
        return;
    }

    for (auto system : activeSystems_[static_cast<size_t>(_category)]) {
        if (system) {
            system->Run();
        }
    }
}

void SystemRunner::RegisterSystem(const std::string& _systemName, int32_t _priority, bool _activate) {
    auto itr = systems_.find(_systemName);

    if (itr == systems_.end() || !itr->second) {
        auto createdSystem = SystemRegistry::GetInstance()->CreateSystem(_systemName, this->scene_);
        if (!createdSystem) {
            LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
            return;
        }

        createdSystem->SetScene(scene_);

        createdSystem->SetPriority(_priority);
        createdSystem->Initialize();

        systems_[_systemName] = std::move(createdSystem);
    } else {
        LOG_WARN("SystemRunner: System '{}' is already registered.", _systemName);
    }

    if (_activate) {
        ActivateSystem(_systemName);
    }
}

void SystemRunner::UnregisterSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }
    if (itr->second) {
        itr->second->SetScene(nullptr);
        itr->second->SetIsActive(false);
        itr->second->Finalize();
    }

    DeactivateSystem(_systemName);
}

void SystemRunner::ActivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("System not found with name: {}", _systemName);
        return;
    }

    ISystem* system = itr->second.get();
    if (!system) {
        LOG_WARN("System '{}' is nullptr.", _systemName);
        return;
    }
    size_t categoryIndex = static_cast<size_t>(system->GetCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), system) != activeSystems.end()) {
        LOG_WARN("System '{}' is already active in category '{}'.", _systemName, SystemCategoryString[categoryIndex]);
        return;
    }

    system->SetIsActive(true);
    activeSystems.emplace_back(system);
    std::sort(
        activeSystems.begin(),
        activeSystems.end(),
        [](const ISystem* a, const ISystem* b) {
            return a->GetPriority() < b->GetPriority(); // priorityが低い順（降順）
        });
}

void SystemRunner::DeactivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }

    ISystem* system      = itr->second.get();
    size_t categoryIndex = static_cast<size_t>(system->GetCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), system) == activeSystems.end()) {
        LOG_WARN("SystemRunner: System '{}' is not active in category '{}'.", _systemName, SystemCategoryString[categoryIndex]);
        return;
    }

    system->SetIsActive(false);

    if (!activeSystems.empty()) {
        auto it = std::remove(activeSystems.begin(), activeSystems.end(), system);
        if (it != activeSystems.end()) {
            activeSystems.erase(it, activeSystems.end());
        }
    }
}

void SystemRunner::RegisterEntity(const std::string& _systemTypeName, Entity* _entity) {
    // システム名からシステムを取得し、エンティティを登録
    auto systemItr = systems_.find(_systemTypeName);
    if (systemItr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", _systemTypeName);
        return;
    }
    ISystem* system = systemItr->second.get();

    if (system) {
        system->AddEntity(_entity);
    } else {
        LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
    }
}

void SystemRunner::RemoveEntity(const std::string& _systemTypeName, Entity* _entity) {
    // システム名からシステムを取得し、エンティティを登録
    auto systemItr = systems_.find(_systemTypeName);
    if (systemItr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", _systemTypeName);
        return;
    }
    // システム名からシステムを取得し、エンティティを削除
    ISystem* system = systemItr->second.get();
    if (system) {
        system->RemoveEntity(_entity);
    } else {
        LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
    }
}

void SystemRunner::RemoveEntityFromAllSystems(Entity* _entity) {
    // 各システムからエンティティを削除
    for (auto& [name, system] : systems_) {
        if (system) {
            system->RemoveEntity(_entity);
        }
    }
}

ISystem* SystemRunner::GetSystem(const std::string& _systemName) const {
    auto itr = systems_.find(_systemName);
    if (itr != systems_.end()) {
        return itr->second.get();
    }

    LOG_ERROR("SystemRunner: System '{}' not found in any category.", _systemName);
    return nullptr;
}

ISystem* SystemRunner::GetSystemRef(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr != systems_.end()) {
        return itr->second.get();
    }

    LOG_ERROR("SystemRunner: System '{}' not found in any category.", _systemName);
    return nullptr;
}

} // namespace OriGine
