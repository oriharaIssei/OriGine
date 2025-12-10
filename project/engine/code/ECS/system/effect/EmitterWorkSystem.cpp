#include "EmitterWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/effect/particle/emitter/Emitter.h"

using namespace OriGine;

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

    const float deltaTime = Engine::GetInstance()->GetDeltaTime();

    auto emitters = GetComponents<Emitter>(_entity);

    if (!emitters) {
        return;
    }

    for (auto& comp : *emitters) {
        comp.Update(deltaTime);
    }
}
