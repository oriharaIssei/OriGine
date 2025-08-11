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
        return !entity || !entity->isAlive();
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

void ISystem::addComponent(GameEntity* _entity, const std::string& _typeName, IComponent* _component, bool _doInitialize) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not set.");
        return;
    }
    scene_->getComponentRepositoryRef()->getComponentArray(_typeName)->addComponent(_entity, _component, _doInitialize);
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
        system->setScene(nullptr);
        system->setIsActive(false);
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
            system->Update();
        }
    }
}

void SystemRunner::registerSystem(const std::string& _systemName, int32_t _priority, bool _isInitialize, bool _activate) {
    auto itr = systems_.find(_systemName);

    if (itr == systems_.end()) {
        auto createdSystem = SystemRegistry::getInstance()->createSystem(_systemName, this->scene_);
        if (!createdSystem) {
            LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
            return;
        }

        createdSystem->setPriority(_priority);

        if (_isInitialize) {
            createdSystem->Initialize();
        }

        systems_[_systemName] = std::move(createdSystem);
    } else {
        LOG_WARN("SystemRunner: System '{}' is already registered.", _systemName);
    }

    if (_activate) {
        ActivateSystem(_systemName);
    }
}

void SystemRunner::unregisterSystem(const std::string& _systemName, bool _isFinalize) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }
    if (itr->second) {
        itr->second->setScene(nullptr);
        itr->second->setIsActive(false);
        if (_isFinalize) {
            itr->second->Finalize();
        }
    }

    DeactivateSystem(_systemName);
}

void SystemRunner::ActivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }

    ISystem* system      = itr->second.get();
    size_t categoryIndex = static_cast<size_t>(system->getCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), system) != activeSystems.end()) {
        LOG_WARN("SystemRunner: System '{}' is already active in category '{}'.", _systemName, SystemCategoryString[categoryIndex]);
        return;
    }

    system->setIsActive(true);
    activeSystems.emplace_back(system);
    std::sort(
        activeSystems.begin(),
        activeSystems.end(),
        [](const ISystem* a, const ISystem* b) {
            return a->getPriority() < b->getPriority(); // priorityが低い順（降順）
        });
}

void SystemRunner::DeactivateSystem(const std::string& _systemName) {
    auto itr = systems_.find(_systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System not found with name: {}", _systemName);
        return;
    }

    ISystem* system      = itr->second.get();
    size_t categoryIndex = static_cast<size_t>(system->getCategory());
    auto& activeSystems  = activeSystems_[categoryIndex];
    if (std::find(activeSystems.begin(), activeSystems.end(), system) == activeSystems.end()) {
        LOG_WARN("SystemRunner: System '{}' is not active in category '{}'.", _systemName, SystemCategoryString[categoryIndex]);
        return;
    }

    system->setIsActive(false);

    if (!activeSystems.empty()) {
        auto it = std::remove(activeSystems.begin(), activeSystems.end(), system);
        if (it != activeSystems.end()) {
            activeSystems.erase(it, activeSystems.end());
        }
    }
}

#pragma endregion
