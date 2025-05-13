#include "ECSManager.h"

/// engine
// module
#include "camera/CameraManager.h"
#include "module/editor/EditorGroup.h"
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
    ExecuteEntitiesDelete();

#ifdef _DEBUG
    if (SceneManager::getInstance()->inDebugMode()) {
        // DebugState == Play の場合のみ 更新
        if (SceneManager::getInstance()->debugIsPlay()) {
            // システムの更新
            for (auto& system : workSystems_[int32_t(SystemType::Input)]) {
                system->Update();
            }
            for (auto& system : workSystems_[int32_t(SystemType::StateTransition)]) {
                system->Update();
            }
            for (auto& system : workSystems_[int32_t(SystemType::Movement)]) {
                system->Update();
            }
            for (auto& system : workSystems_[int32_t(SystemType::Collision)]) {
                system->Update();
            }
            for (auto& system : workSystems_[int32_t(SystemType::Effect)]) {
                system->Update();
            }
        }
    }

    if (!SceneManager::getInstance()->isUsingDebugCamera()) {
        CameraManager::getInstance()->DataConvertToBuffer();
    }

    auto sceneView = SceneManager::getInstance()->getSceneView();

    sceneView->PreDraw();

    for (auto& system : workSystems_[int32_t(SystemType::Render)]) {
        system->Update();
    }

    sceneView->PostDraw();

    for (auto& system : workSystems_[int32_t(SystemType::PostRender)]) {
        system->Update();
    }

#else
    // システムの更新
    for (auto& system : workSystems_[int32_t(SystemType::Input)]) {
        system->Update();
    }
    for (auto& system : workSystems_[int32_t(SystemType::StateTransition)]) {
        system->Update();
    }
    for (auto& system : workSystems_[int32_t(SystemType::Movement)]) {
        system->Update();
    }
    for (auto& system : workSystems_[int32_t(SystemType::Collision)]) {
        system->Update();
    }
    for (auto& system : workSystems_[int32_t(SystemType::Effect)]) {
        system->Update();
    }

    CameraManager::getInstance()->DataConvertToBuffer();

    auto sceneView = SceneManager::getInstance()->getSceneView();

    sceneView->PreDraw();
    for (auto& system : workSystems_[int32_t(SystemType::Render)]) {
        system->Update();
    }
    sceneView->PostDraw();

    for (auto& system : workSystems_[int32_t(SystemType::PostRender)]) {
        system->Update();
    }
#endif // _DEBUG
}

void EntityComponentSystemManager::Finalize() {
    // システムのクリア
    FinalizeWorkSystems();
    clearAllSystems();

    // コンポーネントのクリア
    FinalizeComponentArrays();
    clearComponentArrays();

    // エンティティのクリア
    entities_.clear();
    freeEntityIndex_.clear();
}

void EntityComponentSystemManager::ComponentArraysInitialize() {
    for (auto& [componentID, componentArray] : componentArrays_) {
        componentArray->Initialize(entityCapacity_);
    }
}

void EntityComponentSystemManager::RunInitialize() {
    for (auto& system : workSystems_[int32_t(SystemType::Initialize)]) {
        if (!system->isActive()) {
            continue;
        }
        system->Update();
    }
}

void EntityComponentSystemManager::ExecuteEntitiesDelete() {
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
            compArray->deleteEntity(entity);
        }
        // システムから除外
        for (auto& systemByType : systems_) {
            for (auto& [name, system] : systemByType) {
                if (system->hasEntity(entity)) {
                    system->removeEntity(entity);
                }
            }
        }

        // エンティティIDを再利用可能にする
        freeEntityIndex_.push_back(entity->getID());

        // エンティティがユニークな場合は、ユニークエンティティから削除
        if (entity->isUnique_) {
            this->removeUniqueEntity(entity->dataType_);
        }
        // エンティティを無効化
        entity->id_       = -1;
        entity->dataType_ = "UNKNOWN";
        entity->setIsAlive(false);
    }
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

bool EntityComponentSystemManager::ActivateSystem(const std::string& _name, bool _doInit) {
    for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
        auto itr = systems_[i].find(_name);

        if (itr != systems_[i].end()) {
            auto workkingSystem = itr->second.get();
            workkingSystem->setIsActive(true);

            workSystems_[i].push_back(workkingSystem);
            if (_doInit) {
                workSystems_[i].back()->Initialize();
            }
            std::sort(workSystems_[i].begin(), workSystems_[i].end(),
                [](ISystem* a, ISystem* b) {
                    return a->getPriority() < b->getPriority();
                });
            return true;
        }
    }

    // システムが見つからない場合は、falseを返す
    return false;
}

bool EntityComponentSystemManager::ActivateSystem(const std::string& _name, SystemType _type, bool _doInit) {
    int32_t systemTypeIndex = int32_t(_type);
    auto itr                = systems_[systemTypeIndex].find(_name);
    if (itr == systems_[systemTypeIndex].end()) {
        // システムが見つからない場合は、falseを返す
        return false;
    }
    auto workkingSystem = itr->second.get();
    workkingSystem->setIsActive(true);

    workSystems_[systemTypeIndex].push_back(workkingSystem);
    if (_doInit) {
        workSystems_[systemTypeIndex].back()->Initialize();
    }
    std::sort(workSystems_[systemTypeIndex].begin(), workSystems_[systemTypeIndex].end(),
        [](ISystem* a, ISystem* b) {
            return a->getPriority() < b->getPriority();
        });

    return true;
}

void EntityComponentSystemManager::AllActivateSystem(bool _doInit) {
    for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
        if (!workSystems_[i].empty()) {
            workSystems_[i].clear();
        }
        for (auto& [name, system] : systems_[i]) {
            ActivateSystem(name, static_cast<SystemType>(i), _doInit);
        }
    }
}

void DestroyEntity(GameEntity* _entity) {
    ECSManager::getInstance()->destroyEntity(_entity);
}
