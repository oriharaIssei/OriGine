#include "ECSManager.h"

/// engine
// module
#include "./application/scene/manager/SceneManager.h"
#include "camera/CameraManager.h"
#include "editor/EngineEditor.h"
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
#ifdef _DEBUG

    if (EngineEditor::getInstance()->isActive()) {
        // Debug,Edit 用システム追加予定
        CameraManager::getInstance()->DebugUpdate();
    } else {
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

#else
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
#endif // _DEBUG
}

void EntityComponentSystemManager::Finalize() {
    // エンティティのクリア
    entities_.clear();
    freeEntityIndex_.clear();
    // コンポーネントのクリア
    for (auto& [componentID, componentArray] : componentArrays_) {
        componentArray->clear();
    }
    // システムのクリア
    clearSystem();
}

void EntityComponentSystemManager::ComponentArraysInit() {
    for (auto& [componentID, componentArray] : componentArrays_) {
        componentArray->Init(entityCapacity_);
    }
}
