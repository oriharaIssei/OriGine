#include "DissolveAnimationSystem.h"

/// engine
#define DELTA_TIME
#include "engine/EngineInclude.h"

using namespace OriGine;

DissolveAnimationSystem::DissolveAnimationSystem() : ISystem(SystemCategory::Effect) {}
OriGine::DissolveAnimationSystem::~DissolveAnimationSystem() {}

void OriGine::DissolveAnimationSystem::Initialize() {}
void OriGine::DissolveAnimationSystem::Finalize() {}

void OriGine::DissolveAnimationSystem::UpdateEntity(EntityHandle _handle) {
    auto* dissolveAnimComp  = GetComponent<DissolveAnimation>(_handle);
    auto* dissolveParamComp = GetComponent<DissolveEffectParam>(_handle);

    if (dissolveAnimComp == nullptr || dissolveParamComp == nullptr) {
        return;
    }

    const float deltaTime = GetMainDeltaTime();
    dissolveAnimComp->Update(deltaTime, dissolveParamComp);
}
