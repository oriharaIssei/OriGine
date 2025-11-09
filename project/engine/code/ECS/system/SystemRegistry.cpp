#include "SystemRegistry.h"

std::unique_ptr<ISystem> SystemRegistry::CreateSystem(const std::string& _systemTypeName, Scene* scene) {
    auto itr = systemMaker_.find(_systemTypeName);
    if (itr == systemMaker_.end()) {
        LOG_ERROR("SystemRegistry: System '{}' not found.", _systemTypeName);
        return nullptr;
    }
    auto system = itr->second(scene);
    if (system) {
        return std::move(system);
    } else {
        LOG_ERROR("SystemRegistry: Failed to create system '{}'.", _systemTypeName);
        return nullptr;
    }
}
