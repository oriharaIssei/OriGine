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
    entities_.clear();
}

void EmitterWorkSystem::UpdateEntity(EntityHandle _handle) {
    const float deltaTime = Engine::GetInstance()->GetDeltaTime();

    auto&emitters = GetComponents<Emitter>(_handle);

    if (emitters.empty()) {
        return;
    }

    for (auto& comp : emitters) {
        comp.Update(deltaTime);
    }
}
