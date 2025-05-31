#include "ECSDebugger.h"

#ifdef _DEBUG

/// externals
#include "imgui/imgui.h"

#pragma region "EntityDebugger"
EntityDebugger::EntityDebugger() : IDebugger() {
    debugEntity_ = nullptr;
    ecsManager_  = nullptr;
}

EntityDebugger::~EntityDebugger() {
}

void EntityDebugger::Initialize() {
    ecsManager_  = ECSManager::getInstance();
    debugEntity_ = nullptr;
    debugEntityComponents_.clear();
}

void EntityDebugger::Update() {
    ImGui::Begin("Entity Debugger");
    /// ===================================
    /// Active Entity List
    /// ===================================
    ImGui::Text("Entity Capacity :%d", ecsManager_->getEntityCapacity());
    ImGui::Text("Active Entity Count: %d", ecsManager_->getActiveEntityCount());

    ImGui::Text("Active Entity List");
    ImGui::Separator();
    for (auto& entity : ecsManager_->getEntities()) {
        if (!entity.isAlive()) {
            continue;
        }
        if (debugEntity_ == &entity) {
            ImGui::Text("%s", entity.isUnique() ? entity.getDataType().c_str() : entity.getUniqueID().c_str());

        } else if (ImGui::Button(entity.isUnique() ? entity.getDataType().c_str() : entity.getUniqueID().c_str())) {
            debugEntity_ = const_cast<GameEntity*>(&entity);
            debugEntityComponents_.clear();
            for (auto& [componentName, compList] : ecsManager_->getComponentArrayMap()) {
                if (compList->hasEntity(debugEntity_)) {
                    auto& compListRef = debugEntityComponents_[componentName];

                    for (int32_t i = 0; i < compList->getComponentSize(debugEntity_); ++i) {
                        auto comp = compList->getComponent(debugEntity_, i);
                        if (comp == nullptr) {
                            continue;
                        }
                        compListRef.push_back(comp);
                    }
                }
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Entity Info");
    if (debugEntity_ != nullptr && debugEntity_->isAlive()) {
        ImGui::Separator();
        ImGui::Text("Entity Info:");
        ImGui::Text("UniqueID : %s", debugEntity_->getUniqueID().c_str());
        ImGui::Text("ID       : %d", debugEntity_->getID());
        // 各コンポーネントの値を確認するためのUIを追加

        std::string componentLabel;

        for (auto& [compTypeName, compList] : debugEntityComponents_) {
            for (size_t i = 0; i < compList.size(); ++i) {
                componentLabel = compTypeName + "[" + std::to_string(i) + "]";
                if (ImGui::TreeNode(compTypeName.c_str())) {
                    compList[i]->Debug();
                    ImGui::TreePop();
                }
            }
        }
    } else {
        ImGui::Text("No Entity Selected");
    }

    ImGui::End();
}

void EntityDebugger::Finalize() {
    debugEntity_ = nullptr;
    debugEntityComponents_.clear();
}
#pragma endregion

#pragma region "SystemDebugger"
SystemDebugger::SystemDebugger() : IDebugger() {
    ecsManager_ = nullptr;
}
SystemDebugger::~SystemDebugger() {
}
void SystemDebugger::Initialize() {
    ecsManager_ = ECSManager::getInstance();

    SortPriorityOrderFromECSManager();
}

void SystemDebugger::SortPriorityOrderFromECSManager() {
    const auto& systemsArray = ecsManager_->getSystems();
    int32_t systemTypeIndex  = 0;
    for (const auto& sysMap : systemsArray) {
        workSystemList_[systemTypeIndex].clear();
        for (const auto& [sysName, sysPtr] : sysMap) {
            // システム名とそのシステムに登録されているエンティティのリストを追加
            workSystemList_[int32_t(sysPtr->getSystemType())].push_back(std::make_pair(sysName, sysPtr.get()));
        }

        std::sort(
            workSystemList_[systemTypeIndex].begin(),
            workSystemList_[systemTypeIndex].end(),
            [](const std::pair<std::string, ISystem*>& a,
                const std::pair<std::string, ISystem*>& b) {
                return a.second->getPriority() < b.second->getPriority();
            });

        systemTypeIndex++;
    }
}

void SystemDebugger::SortPriorityOrderSystems(int32_t _systemTypeIndex) {
    workSystemList_[_systemTypeIndex].clear();

    const auto& systemMap = ecsManager_->getSystemsBy(SystemType(_systemTypeIndex));
    for (const auto& [sysName, sysPtr] : systemMap) {
        // システム名とそのシステムに登録されているエンティティのリストを追加
        workSystemList_[int32_t(sysPtr->getSystemType())].push_back(std::make_pair(sysName, sysPtr.get()));
    }

    std::sort(
        workSystemList_[_systemTypeIndex].begin(),
        workSystemList_[_systemTypeIndex].end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });
}

void SystemDebugger::Update() {
    if (ImGui::Begin("System Debugger")) {
        ImGui::Text("System List");
        ImGui::Separator();
        std::string systemLabel;
        for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                SortPriorityOrderSystems(systemTypeIndex);

                ImGui::Indent();
                for (auto& [name, system] : workSystemList_[systemTypeIndex]) {
                    systemLabel   = "##" + name + "isActive";
                    bool isActive = system->isActive();
                    ImGui::Checkbox(systemLabel.c_str(), &isActive);
                    system->setIsActive(isActive);

                    ImGui::SameLine();
                    if (ImGui::CollapsingHeader(name.c_str())) {
                        ImGui::Indent();
                        ImGui::Text("Name        : %s", name.c_str());
                        ImGui::Text("System Type : %s", SystemTypeString[int32_t(system->getSystemType())].c_str());
                        ImGui::Text("Priority    : %d", system->getPriority());
                        ImGui::Text("Entity Count: %d", system->getEntityCount());
                        ImGui::Text("Running Time  : %.3f / ms", system->getRunningTime());

                        ImGui::Separator();
                        ImGui::Text("Entities    :");
                        for (auto& id : system->getEntityIDs()) {
                            GameEntity* entity = ecsManager_->getEntity(id);
                            if (entity == nullptr) {
                                continue;
                            }
                            ImGui::Text("%s", entity->getUniqueID().c_str());
                        }
                        ImGui::Unindent();
                    }
                }
                ImGui::Unindent();
            }
        }
    }
    ImGui::End();
}

void SystemDebugger::Finalize() {
}

#pragma endregion

#endif // _DEBUG
