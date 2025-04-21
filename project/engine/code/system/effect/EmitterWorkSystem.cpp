#include "EmitterWorkSystem.h"

/// engine
// ECS
#include "ECSManager.h"
// component
#include "component/effect/particle/emitter/Emitter.h"

#include "Engine.h"

void EmitterWorkSystem::Initialize() {
}
void EmitterWorkSystem::Finalize() {
    entities_.clear();
}

void EmitterWorkSystem::UpdateEntity(GameEntity* _entity) {
    // _entityがnullptrの場合は何もしない
    if (!_entity) {
        return;
    }

    const float deltaTime = Engine::getInstance()->getDeltaTime();

    for (auto& comp : *getComponents<Emitter>(_entity)) {
        comp.Update(deltaTime);
    }
}
