#include "ISystem.h"

void ISystem::Update() {
    std::erase_if(entities_, [](GameEntity* _entity) {
        return !_entity || !_entity->isAlive();
    });

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }
}
