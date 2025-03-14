#include "ECSManager.h"

/// engine
// module
#include "camera/CameraManager.h"
#include "editor/EngineEditor.h"
#include "sceneManager/SceneManager.h"
// dx12Object
#include "engine/directX12/RenderTexture.h"

void EntityComponentSystemManager::Initialize() {
    // エンティティの初期化
    if (!entities_.empty()) {
        entities_.clear();
    }
    entities_.resize(entityCapacity_);
    for (uint32_t i = 0; i < entityCapacity_; ++i) {
        freeEntityIndex_.push_back(i);
    }
}

void EntityComponentSystemManager::Run() {
    /// =================================================================================================
    // 削除予定のエンティティを削除
    /// =================================================================================================
    while (!deleteEntityQueue_.empty()) {
        GameEntity* entity = deleteEntityQueue_.front();
        deleteEntityQueue_.pop();

        for (auto& [compTypeName, compArray] : componentArrays_) {
            compArray->clearComponent(entity);
        }

        entity->setAlive(false); // エンティティを無効化
        freeEntityIndex_.push_back(entity->getID()); // エンティティIDを再利用可能にする
    }

#ifdef _DEBUG
    if (EngineEditor::getInstance()->isActive()) {
        // Debug,Edit 用システム追加予定
        CameraManager::getInstance()->DebugUpdate();
    } else {
        // システムの更新
        for (auto& system : priorityOrderSystems_[int32_t(SystemType::Input)]) {
            system->Update();
        }
        for (auto& system : priorityOrderSystems_[int32_t(SystemType::StateTransition)]) {
            system->Update();
        }
        for (auto& system : priorityOrderSystems_[int32_t(SystemType::Movement)]) {
            system->Update();
        }
        for (auto& system : priorityOrderSystems_[int32_t(SystemType::Physics)]) {
            system->Update();
        }
        for (auto& system : priorityOrderSystems_[int32_t(SystemType::Collision)]) {
            system->Update();
        }
    }

    auto sceneView = SceneManager::getInstance()->getSceneView();
    sceneView->PreDraw();
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Render)]) {
        system->Update();
    }
    sceneView->PostDraw();

    for (auto& system : priorityOrderSystems_[int32_t(SystemType::PostRender)]) {
        system->Update();
    }

#else
    // システムの更新
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Input)]) {
        system->Update();
    }
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::StateTransition)]) {
        system->Update();
    }
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Movement)]) {
        system->Update();
    }
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Physics)]) {
        system->Update();
    }
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Collision)]) {
        system->Update();
    }

    auto sceneView = SceneManager::getInstance()->getSceneView();
    sceneView->PreDraw();
    for (auto& system : priorityOrderSystems_[int32_t(SystemType::Render)]) {
        system->Update();
    }
    sceneView->PostDraw();

    for (auto& system : priorityOrderSystems_[int32_t(SystemType::PostRender)]) {
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

void EntityComponentSystemManager::ComponentArraysInitialize() {
    for (auto& [componentID, componentArray] : componentArrays_) {
        componentArray->Initialize(entityCapacity_);
    }
}

void DestroyEntity(GameEntity* _entity) {
    ECSManager::getInstance()->destroyEntity(_entity);
}
