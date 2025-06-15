#include "ECSManager.h"

/// engine
// module
#include "camera/CameraManager.h"
#include "module/editor/EditorController.h"
#include "sceneManager/SceneManager.h"
// dx12Object
#include "engine/code/directX12/RenderTexture.h"

/// externals
#include "imgui/imgui.h"
#include "logger/Logger.h"

void EntityComponentSystemManager::Initialize() {
    // エンティティの初期化
    if (!entityes_.empty()) {
        entityes_.clear();
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
    entityes_.clear();
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
        uint32_t deleteID = deleteEntityQueue_.front();
        deleteEntityQueue_.pop();

        GameEntity* entity = &entityes_[deleteID];

        // 無効ポインタにアクセスしないように 弾く
        if (!entity || !entity->isAlive()) {
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
                system->removeEntity(entity);
            }
        }

        // エンティティIDを再利用可能にする
        int32_t entityID = entity->getID();
        if (entityID < 0) {
            LOG_CRITICAL("The entity ID is invalid. \n EntityName : {} ", entity->dataType_);
            continue;
        }

        freeEntityIndex_.insert(freeEntityIndex_.end() - entity->getID(), entity->getID());

        // エンティティがユニークな場合は、ユニークエンティティから削除
        if (entity->isUnique_) {
            this->removeUniqueEntity(entity->dataType_);
        }
        // エンティティを無効化
        entity->id_       = -1;
        entity->dataType_ = "UNKNOWN";
        entity->isAlive_  = false;
    }
}

void EntityComponentSystemManager::resize(uint32_t _newSize) {
    uint32_t oldSize = static_cast<uint32_t>(entityes_.size());

    if (_newSize <= oldSize) {
        // 必要のないものを削除
        std::erase_if(entityes_, [](const GameEntity& entity) { return !entity.isAlive_; });

        // エンティティの使用容量より _newSize が小さい場合
        if (entityes_.size() >= _newSize) {
            assert(false);
        }

        // エンティティのIDを振り直す
        uint32_t entityIndex = 0;
        for (auto& entity : entityes_) {
            entity.id_ = entityIndex;
            ++entityIndex;
        }

        // freeEntityIndexを再構築
        freeEntityIndex_.clear();
        for (uint32_t i = _newSize - 1;; --i) {
            freeEntityIndex_.push_back(i);
            if (i == 0)
                break;
        }
    } else {
        // エンティティの容量を増やす
        entityes_.resize(_newSize);
        for (uint32_t i = _newSize - 1;; --i) {
            freeEntityIndex_.push_back(i);
            if (i == oldSize)
                break;
        }
    }
    entityCapacity_ = _newSize;
}

uint32_t EntityComponentSystemManager::registerEntity(const std::string& _entityDataType) {
    if (freeEntityIndex_.empty()) {
        // 容量に空きが無い場合, 容量を増やす
        // 2倍の容量にリサイズ
        resize(entityCapacity_ * 2);

        for (auto& [componentID, componentArray] : componentArrays_) {
            componentArray->Initialize(entityCapacity_);
        }
    }
    uint32_t index = freeEntityIndex_.back();
    freeEntityIndex_.pop_back();

    entityes_[index]          = GameEntity(_entityDataType, index);
    entityes_[index].isAlive_ = true;

    return index;
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

GameEntity* getEntity(int32_t _entityIndex) {
    return ECSManager::getInstance()->getEntity(_entityIndex);
}

nlohmann::json EntityToJson(GameEntity* _entity) {
    nlohmann::json json = nlohmann::json::object();
    json["Name"]        = _entity->getDataType();
    json["isUnique"]    = _entity->isUnique();

    // 所属するシステムを保存
    const auto& systems = ECSManager::getInstance()->getSystems();
    for (const auto& systemsByType : systems) {
        for (const auto& [systemName, system] : systemsByType) {
            if (system->hasEntity(_entity)) {
                json["Systems"].push_back({{"SystemType", system->getSystemType()}, {"SystemName", systemName}});
            }
        }
    }

    // コンポーネントを保存
    const auto& componentArrayMap = ECSManager::getInstance()->getComponentArrayMap();
    nlohmann::json componentsData;
    for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
        if (componentArray->hasEntity(_entity)) {
            componentArray->SaveComponent(_entity, componentsData);
        }
    }
    json["Components"] = componentsData;

    return json;
}

GameEntity* EntityFromJson(const nlohmann::json& _json) {
    ECSManager* ecsManager = ECSManager::getInstance();
    std::string entityName = _json["Name"];
    int32_t entityID       = ecsManager->registerEntity(entityName);
    GameEntity* entity     = ecsManager->getEntity(entityID);
    bool isUnique          = _json["isUnique"];
    if (isUnique) {
        ecsManager->registerUniqueEntity(entity);
    }
    // 所属するシステムを読み込み
    for (auto& systemData : _json["Systems"]) {
        int32_t systemType     = systemData["SystemType"];
        std::string systemName = systemData["SystemName"];
        ISystem* system        = ecsManager->getSystem(SystemType(systemType), systemName);
        if (system) {
            system->addEntity(entity);
        }
    }
    // コンポーネントを読み込み
    auto& componentArrayMap = ecsManager->getComponentArrayMap();
    for (auto& [componentTypename, componentData] : _json["Components"].items()) {
        auto itr = componentArrayMap.find(componentTypename);
        if (itr != componentArrayMap.end()) {
            itr->second->LoadComponent(entity, componentData);
        }
    }
    return entity;
}

void DestroyEntity(GameEntity* _entity) {
    ECSManager::getInstance()->destroyEntity(_entity);
}
