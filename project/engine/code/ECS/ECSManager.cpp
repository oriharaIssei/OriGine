#include "ECSManager.h"

/// engine
// module
#include "camera/CameraManager.h"
#include "editor/EngineEditor.h"
#include "sceneManager/SceneManager.h"
// dx12Object
#include "engine/code/directX12/RenderTexture.h"

/// externals
#include "imgui/imgui.h"

void EntityComponentSystemManager::Initialize() {
    // エンティティの初期化
    if (!entities_.empty()) {
        entities_.clear();
    }
    if (!freeEntityIndex_.empty()) {
        freeEntityIndex_.clear();
    }

    resize(entityCapacity_);
}

void EntityComponentSystemManager::Run() {
    /// =================================================================================================
    // 削除予定のエンティティを削除
    /// =================================================================================================
    while (!deleteEntityQueue_.empty()) {
        GameEntity* entity = deleteEntityQueue_.front();
        deleteEntityQueue_.pop();

        // 無効ポインタにアクセスしないように 弾く
        if (!entity) {
            continue;
        }

        // component の 初期化
        for (auto& [compTypeName, compArray] : componentArrays_) {
            compArray->clearComponent(entity);
        }

        // エンティティIDを再利用可能にする
        freeEntityIndex_.push_back(entity->getID()); 

        // エンティティを無効化
        entity->id_ = -1;
        entity->dataType_ = "";
        entity->setAlive(false);
    }

#ifdef _DEBUG
    if (EngineEditor::getInstance()->isActive()) {
        // Debug,Edit 用システム追加予定
        CameraManager::getInstance()->DebugUpdate();
    } else {

        ShowEntityStack();

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

void EntityComponentSystemManager::ShowEntityStack() {
    ImGui::Begin("EntityStack");
    for (auto& entity : entities_) {
        if (entity.isAlive_) {
            ImGui::Text("EntityID : %d", entity.getID());
        }
    }
    ImGui::End();
}

void EntityComponentSystemManager::resize(uint32_t _newSize) {
    uint32_t oldSize = static_cast<uint32_t>(entities_.size());

    if (_newSize <= oldSize) {
        // 必要のないものを削除
        std::erase_if(entities_, [](const GameEntity& entity) { return !entity.isAlive_; });

        // エンティティの使用容量より _newSize が小さい場合
        if (entities_.size() >= _newSize) {
            assert(false);
        }

        // エンティティのIDを振り直す
        uint32_t entityIndex = 0;
        for (auto& entity : entities_) {
            entity.id_ = entityIndex;
            ++entityIndex;
        }

        // freeEntityIndexを再構築
        freeEntityIndex_.clear();
        for (uint32_t i = 0; i < _newSize; i++) {
            freeEntityIndex_.push_back(i);
        }
    } else {
        // エンティティの容量を増やす
        entities_.resize(_newSize);

        for (uint32_t i = oldSize; i < _newSize; ++i) {
            freeEntityIndex_.push_back(i);
        }
    }
    entityCapacity_ = _newSize;
}

void DestroyEntity(GameEntity* _entity) {
    ECSManager::getInstance()->destroyEntity(_entity);
}
