#include "SystemRunner.h"

/// ECS
// system
#include "SystemRegistry.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="_scene">所属させるScene</param>
SystemRunner::SystemRunner(Scene* _scene) : scene_(_scene) {}
/// <summary>
/// デストラクタ
/// </summary>
SystemRunner::~SystemRunner() {}

/// <summary>
/// 全てのカテゴリのシステムを初期化する
/// </summary>
void SystemRunner::InitializeAllCategory() {
    for (auto& [name, system] : systems_) {
        system->Initialize();
    }
}

/// <summary>
/// 指定したカテゴリのシステムを初期化する
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
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

/// <summary>
/// アクティブなシステムをすべて初期化する (非推奨)
/// </summary>
void SystemRunner::InitializeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto system : systemByCategory) {
            if (system) {
                system->Initialize();
            }
        }
    }
}

/// <summary>
/// 指定したカテゴリのアクティブなシステムを初期化する (非推奨)
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
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

/// <summary>
/// 全てのカテゴリのシステムを終了処理する
/// </summary>
void SystemRunner::FinalizeAllCategory() {
    for (auto& [systemName, system] : systems_) {
        if (system) {
            system->Finalize();
        }
    }
}

/// <summary>
/// 指定したカテゴリのシステムを終了処理する
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
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

/// <summary>
/// アクティブなシステムをすべて終了処理する (非推奨)
/// </summary>
void SystemRunner::FinalizeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto system : systemByCategory) {
            if (system) {
                system->Finalize();
            }
        }
    }
}

/// <summary>
/// 指定したカテゴリのアクティブなシステムを終了処理する (非推奨)
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
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

/// <summary>
/// 全てのシステムの登録を解除する
/// </summary>
/// <param name="_isFinalize">解除時に終了処理を行うか</param>
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

/// <summary>
/// 指定したカテゴリのシステムを更新する
/// </summary>
/// <param name="_category">対象のカテゴリ</param>
void SystemRunner::UpdateCategory(SystemCategory _category) {
    if (!categoryActivity[static_cast<size_t>(_category)]) {
        return;
    }

    for (size_t i = 0; i < activeSystems_[static_cast<size_t>(_category)].size(); ++i) {
        auto& system = activeSystems_[static_cast<size_t>(_category)][i];
        if (system) {
            system->Run();
        }
    }
}

/// <summary>
/// システムの名前を指定して登録する
/// </summary>
/// <param name="_systemName">システム名</param>
/// <param name="_priority">優先度</param>
/// <param name="_activate">アクティブにするか</param>
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

/// <summary>
/// システムの登録を解除する
/// </summary>
/// <param name="_systemName">システム名</param>
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

/// <summary>
/// システムをアクティブにする
/// </summary>
/// <param name="_systemName">システム名</param>
void SystemRunner::ActivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("System not found with name: {}", _systemName);
        return;
    }

    if (!itr->second) {
        LOG_WARN("System '{}' is nullptr.", _systemName);
        return;
    }
    size_t categoryIndex = static_cast<size_t>(itr->second->GetCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), itr->second) != activeSystems.end()) {
        LOG_WARN("System '{}' is already active in category '{}'.", _systemName, kSystemCategoryString[categoryIndex]);
        return;
    }

    itr->second->SetIsActive(true);
    activeSystems.push_back(itr->second);
    std::sort(
        activeSystems.begin(),
        activeSystems.end(),
        [](const std::shared_ptr<ISystem>& a, const std::shared_ptr<ISystem>& b) {
            return a->GetPriority() < b->GetPriority(); // priorityが低い順（降順）
        });
}

/// <summary>
/// システムを非アクティブにする
/// </summary>
/// <param name="_systemName">システム名</param>
void SystemRunner::DeactivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("System not found with name: {}", _systemName);
        return;
    }

    ::std::shared_ptr<ISystem> system = itr->second;
    if (!system) {
        LOG_ERROR("System is empty with name: {}", _systemName);
        return;
    }

    size_t categoryIndex = static_cast<size_t>(system->GetCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), itr->second) == activeSystems.end()) {
        LOG_WARN("SystemRunner: System '{}' is not active in category '{}'.", _systemName, kSystemCategoryString[categoryIndex]);
        return;
    }

    itr->second->SetIsActive(false);

    if (!activeSystems.empty()) {
        auto it = std::remove(activeSystems.begin(), activeSystems.end(), itr->second);
        if (it != activeSystems.end()) {
            activeSystems.erase(it, activeSystems.end());
        }
    }
}

/// <summary>
/// 指定したシステムにエンティティを登録する
/// </summary>
/// <param name="_systemTypeName">対象のシステム名</param>
/// <param name="_handle">対象のエンティティハンドル</param>
void SystemRunner::RegisterEntity(const std::string& _systemTypeName, EntityHandle _handle) {
    // システム名からシステムを取得し、エンティティを登録
    auto systemItr = systems_.find(_systemTypeName);
    if (systemItr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", _systemTypeName);
        return;
    }

    if (systemItr->second.get()) {
        systemItr->second.get()->AddEntity(_handle);
    } else {
        LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
    }
}

/// <summary>
/// 指定したシステムからエンティティを削除する
/// </summary>
/// <param name="_systemTypeName">対象のシステム名</param>
/// <param name="_handle">対象のエンティティハンドル</param>
void SystemRunner::RemoveEntity(const std::string& _systemTypeName, EntityHandle _handle) {
    // システム名からシステムを取得し、エンティティを登録
    auto systemItr = systems_.find(_systemTypeName);
    if (systemItr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", _systemTypeName);
        return;
    }
    // システム名からシステムを取得し、エンティティを削除
    if (systemItr->second) {
        systemItr->second->RemoveEntity(_handle);
    } else {
        LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
    }
}

/// <summary>
/// 全てのシステムから指定したエンティティを削除する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void SystemRunner::RemoveEntityFromAllSystems(EntityHandle _handle) {
    // 各システムからエンティティを削除
    for (auto& [name, system] : systems_) {
        if (system) {
            system->RemoveEntity(_handle);
        }
    }
}

/// <summary>
/// システム名からシステムを取得する
/// </summary>
/// <param name="_systemName">システム名</param>
/// <returns>システムの共有ポインタ</returns>
::std::shared_ptr<ISystem> SystemRunner::GetSystem(const std::string& _systemName) const {
    auto itr = systems_.find(_systemName);
    if (itr != systems_.end()) {
        return itr->second;
    }

    LOG_ERROR("SystemRunner: System '{}' not found in any category.", _systemName);
    return nullptr;
}

/// <summary>
/// システム名からシステムのリファレンス（共有ポインタ）を取得する
/// </summary>
/// <param name="_systemName">システム名</param>
/// <returns>システムの共有ポインタ</returns>
::std::shared_ptr<ISystem> SystemRunner::GetSystemRef(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr != systems_.end()) {
        return itr->second;
    }

    LOG_ERROR("SystemRunner: System '{}' not found in any category.", _systemName);
    return nullptr;
}
