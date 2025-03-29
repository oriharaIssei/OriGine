#include "ParticleUpdateSystem.h"

/// engine
#include "Engine.h"
// ECS
#include "ECS/ECSManager.h"

// component
#include "effect/particle/emitter/Emitter.h"

void ParticleUpdateSystem::Initialize() {}

void ParticleUpdateSystem::Finalize() {}

void ParticleUpdateSystem::UpdateEntity(GameEntity* _entity) {
    int32_t currentEmitterIndex = 0;
    while (true) {
        auto emitter = getComponent<Emitter>(_entity, currentEmitterIndex++);
        if (!emitter) {
            return;
        }
        emitter->Update(Engine::getInstance()->getDeltaTime());
    }
}
