#include "ECSManager.h"

/// engine
// module
#include "./application/scene/manager/SceneManager.h"
// dx12Object
#include "engine/directX12/RenderTexture.h"

void EntityComponentSystemManager::Init() {
    // エンティティの初期化
    entities_.resize(entityCapacity_);
    for (uint32_t i = 0; i < entityCapacity_; ++i) {
        freeEntityIndex_.push_back(i);
    }
}

void EntityComponentSystemManager::Run() {
    // システムの更新
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::Input)]) {
        system->Update();
    }
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::StateTransition)]) {
        system->Update();
    }
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::Movement)]) {
        system->Update();
    }
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::Physics)]) {
        system->Update();
    }
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::Collision)]) {
        system->Update();
    }

    auto sceneView = SceneManager::getInstance()->getSceneView();
    sceneView->PreDraw();
    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::Render)]) {
        system->Update();
    }
    sceneView->PostDraw();

    for (auto& [systemTypeName, system] : systems_[int32_t(SystemType::PostRender)]) {
        system->Update();
    }
}
