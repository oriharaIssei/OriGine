#include "ISystem.h"

/// engine
#include "scene/Scene.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

#pragma region "ISystem"
void ISystem::eraseDeadEntity() {
    auto* entityRepository_ = scene_->getEntityRepositoryRef();
    std::erase_if(entityIDs_, [&entityRepository_](int32_t _entityID) {
        GameEntity* entity = entityRepository_->getEntity(_entityID);
        return !entity || _entityID < 0;
    });
}

GameEntity* ISystem::getEntity(int32_t _entityID) {
    if (scene_ == nullptr) {
        LOG_ERROR("Scene is not set.");
        return nullptr;
    }
    return scene_->getEntityRepositoryRef()->getEntity(_entityID);
}

GameEntity* ISystem::getUniqueEntity(const std::string& _dataTypeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not set.");
        return nullptr;
    }
    return scene_->getEntityRepositoryRef()->getUniqueEntity(_dataTypeName);
}

IComponentArray* ISystem::getComponentArray(const std::string& _typeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return scene_->getComponentRepositoryRef()->getComponentArray(_typeName);
}

void ISystem::Update() {
#ifdef _DEBUG
    // 計測開始
    deltaTimer_.Initialize();
#endif

    if (entityIDs_.empty()) {
        return;
    }

    eraseDeadEntity();

    for (auto& entityID : entityIDs_) {
        GameEntity* entity = scene_->getEntityRepositoryRef()->getEntity(entityID);
        UpdateEntity(entity);
    }

#ifdef _DEBUG
    // 計測終了
    deltaTimer_.Update();
    runningTime_ = deltaTimer_.getDeltaTime();
#endif
}

void ISystem::Edit() {
#ifdef _DEBUG
    // GUI表示
    ImGui::Separator();
    ImGui::Text("SystemCategory: %s", SystemCategoryString[static_cast<int>(category_)].c_str());

    ImGui::SetNextItemWidth(78);
    InputGuiCommand("Priority", priority_, "%d");

    ImGui::Text("EntityCount: %d", static_cast<int>(entityIDs_.size()));

    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

    if (ImGui::TreeNode("Entities")) {
        if (ImGui::BeginTable("Entities", 2, tableFlags)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableHeadersRow();

            for (auto& entityID : entityIDs_) {
                GameEntity* entity = scene_->getEntityRepositoryRef()->getEntity(entityID);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", entity->getID());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", entity->getDataType().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

#endif
}
#pragma endregion

#pragma region "SystemRunner"
SystemRunner::SystemRunner(Scene* _scene) : scene_(_scene) {}
SystemRunner::~SystemRunner() {}

void SystemRunner::InitializeAllCategory() {
    for (auto& systemByCategory : systems_) {
        for (auto& [systemName, system] : systemByCategory) {
            if (system) {
                system->Initialize();
            }
        }
    }
}

void SystemRunner::InitializeCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto& [systemName, system] : systems_[static_cast<size_t>(_category)]) {
        if (system) {
            system->Initialize();
        }
    }
}

void SystemRunner::InitializeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto pp_system : systemByCategory) {
            auto* system = *pp_system;
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
    for (auto pp_system : activeSystems_[static_cast<size_t>(_category)]) {
        auto* system = *pp_system;
        if (system) {
            system->Initialize();
        }
    }
}

void SystemRunner::FinalizeAllCategory() {
    for (auto& systemByCategory : systems_) {
        for (auto& [systemName, system] : systemByCategory) {
            if (system) {
                system->Finalize();
            }
        }
    }
}

void SystemRunner::FinalizeCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto& [systemName, system] : systems_[static_cast<size_t>(_category)]) {
        if (system) {
            system->Finalize();
        }
    }
}

void SystemRunner::FinalizeActiveSystems() {
    for (auto& systemByCategory : activeSystems_) {
        for (auto pp_system : systemByCategory) {
            auto* system = *pp_system;
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
    for (auto pp_system : activeSystems_[static_cast<size_t>(_category)]) {
        auto* system = *pp_system;
        if (system) {
            system->Finalize();
        }
    }
}

void SystemRunner::AllUnregisterSystem(bool _isFinalize) {
    for (auto& systemByCategory : systems_) {
        for (auto& [systemName, system] : systemByCategory) {
            if (system) {
                system->setScene(nullptr);
                system->setIsActive(false);
                if (_isFinalize) {
                    system->Finalize();
                }
            }
            auto& activeCategorySystems = activeSystems_[static_cast<size_t>(system->getCategory())];
            activeCategorySystems.erase(std::remove(activeCategorySystems.begin(), activeCategorySystems.end(), &system), activeCategorySystems.end());
        }
        systemByCategory.clear();
    }
}

void SystemRunner::UpdateCategory(SystemCategory _category) {
    if (_category == SystemCategory::Count) {
        LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
    }
    for (auto pp_system : activeSystems_[static_cast<size_t>(_category)]) {
        auto p_system = *pp_system;
        if (p_system) {
            p_system->Update();
        }
    }
}

void SystemRunner::registerSystem(const std::string& _systemName, bool _activate) {
    auto* registerAskSystem = SystemRegistry::getInstance()->getSystem(_systemName);
    if (!registerAskSystem) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }
    ISystem* system = registerAskSystem;

    system->setScene(scene_);
    systems_[static_cast<size_t>(system->getCategory())][_systemName] = system;

    if (_activate) {
        activeSystems_[static_cast<size_t>(system->getCategory())].emplace_back(&systems_[static_cast<size_t>(system->getCategory())][_systemName]);
        system->setIsActive(true);
    }
}

void SystemRunner::unregisterSystem(const std::string& _systemName, bool _isFinalize) {
    for (auto& systemByCategory : systems_) {
        auto itr = systemByCategory.find(_systemName);
        if (itr != systemByCategory.end()) {
            if (itr->second) {
                itr->second->setScene(nullptr);
                itr->second->setIsActive(false);
                if (_isFinalize) {
                    itr->second->Finalize();
                }
            }
            auto& activeCategorySystems = activeSystems_[static_cast<size_t>(itr->second->getCategory())];
            activeCategorySystems.erase(std::remove(activeCategorySystems.begin(), activeCategorySystems.end(), &itr->second), activeCategorySystems.end());
            systemByCategory.erase(itr);
            break;
        }
    }
}

void SystemRunner::ActivateSystem(const std::string& _systemName) {
    auto itr = systems_[static_cast<size_t>(SystemCategory::Count)].find(_systemName);
    if (itr == systems_[static_cast<size_t>(SystemCategory::Count)].end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }
    ISystem* system = itr->second;
    activeSystems_[static_cast<size_t>(system->getCategory())].push_back(&system);
}

void SystemRunner::DeactivateSystem(const std::string& _systemName) {
    auto itr = systems_[static_cast<size_t>(SystemCategory::Count)].find(_systemName);
    if (itr == systems_[static_cast<size_t>(SystemCategory::Count)].end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }
    ISystem* system     = itr->second;
    auto& activeSystems = activeSystems_[static_cast<size_t>(system->getCategory())];
    activeSystems.erase(std::remove(activeSystems.begin(), activeSystems.end(), &system), activeSystems.end());
}

#pragma endregion
