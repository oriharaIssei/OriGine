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
    ImGui::Begin("Entity DebuggerGroup");
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
        }
        if (ImGui::Button(entity.isUnique() ? entity.getDataType().c_str() : entity.getUniqueID().c_str())) {
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
    if (debugEntity_ != nullptr) {
        ImGui::Separator();
        ImGui::Text("Entity Info:");
        ImGui::Text("UniqueID: %s", debugEntity_->getUniqueID().c_str());
        ImGui::Text("ID: %d", debugEntity_->getID());
        // 各コンポーネントの値を確認するためのUIを追加
        for (auto& [componentName, compList] : debugEntityComponents_) {
            if (ImGui::TreeNode(componentName.c_str())) {
                for (size_t i = 0; i < compList.size(); ++i) {
                    compList[i]->Edit();
                }
                ImGui::TreePop();
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
}

void SystemDebugger::Update() {
}

void SystemDebugger::Finalize() {
}

#pragma endregion

#endif // _DEBUG
