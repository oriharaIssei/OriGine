#include "Entity.h"

#include "ECSManager.h"

uint32_t CreateEntity(const std::string& _dataType) {
    ECSManager* ecsManager = ECSManager::getInstance();
    // Indexを取得する(エンティティの登録)
    uint32_t entityIndex = ecsManager->registerEntity(_dataType);

    // dataType 通りに コンポネントとシステムを取得する
    const std::vector<std::pair<std::string, int32_t>>& usingComponentArrayTypes = ecsManager->getComponentTypeByDataType(_dataType);
    const std::vector<std::string>& workSystemTypes                              = ecsManager->getSystemsByDataType(_dataType);

    // エンティティにコンポーネントを登録する
    for (auto [componentTypeName, componentSize] : usingComponentArrayTypes) {
        IComponentArray* componentArray = ecsManager->getComponentArray(componentTypeName);
        componentArray->registerEntity(entityIndex, componentSize);
    }

    GameEntity* entity = ecsManager->getEntity(entityIndex);
    // システムにエンティティを登録する
    for (auto systemType : workSystemTypes) {
        ecsManager->getSystem(systemType)->addEntity(entity);
    }
    return entityIndex;
}

void DestroyEntity(GameEntity* _entity) {
    ECSManager* ecsManager = ECSManager::getInstance();
    // TODO : ここでEntityを破棄する処理を書く

    // dataType 通りに コンポネントとシステムを取得する
    const std::vector<std::pair<std::string, int32_t>>& usingComponentArray = ecsManager->getComponentTypeByDataType(_entity->getDataType());
    const std::vector<std::string>& workSystems                             = ecsManager->getSystemsByDataType(_entity->getDataType());

    // エンティティのコンポーネントを削除する
    for (const auto& [componentTypeName, componentSize] : usingComponentArray) {
        ecsManager->getComponentArray(componentTypeName)->clearComponent(_entity);
    }

    // システムにエンティティを削除する
    for (auto systemTypeName : workSystems) {
        ecsManager->getSystem(systemTypeName)->removeEntity(_entity);
    }

    ecsManager->destroyEntity(_entity->getID());
}
