#include "EmitterWorkSystem.h"

/// engine
// ECS

// component
#include "component/effect/particle/emitter/Emitter.h"

#include "Engine.h"

void EmitterWorkSystem::Initialize() {
}
void EmitterWorkSystem::Finalize() {
    entityIDs_.clear();
}

void EmitterWorkSystem::UpdateEntity(Entity* _entity) {
    // _entityがnullptrの場合は何もしない
    if (!_entity) {
        return;
    }

    const float deltaTime = Engine::getInstance()->getDeltaTime();

    auto emitters = getComponents<Emitter>(_entity);

    if (!emitters) {
        return;
    }

    for (auto& comp : *emitters) {
        comp.Update(deltaTime);
    }
}
