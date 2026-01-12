#include "SystemRegistry.h"

using namespace OriGine;

/// <summary>
/// システムの生成
/// </summary>
/// <param name="_systemTypeName">システム名</param>
/// <param name="scene">所属させるScene</param>
/// <returns>生成されたシステムの共有ポインタ</returns>
std::shared_ptr<ISystem> SystemRegistry::CreateSystem(const std::string& _systemTypeName, Scene* scene) {
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
