#include "TransformAnimationWorkSystem.h"

/// engine
#define DELTA_TIME
#include "EngineInclude.h"

/// ECS
// component
#include "component/animation/TransformAnimation.h"
#include "component/transform/Transform.h"

using namespace OriGine;

void TransformAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    EntityHandle handle    = _entity->GetHandle();
    auto& animations = GetComponents<TransformAnimation>(_entity->GetHandle());
    if (animations.empty()) {
        return;
    }
    const float deltaTime = GetMainDeltaTime();

    for (auto& transAnim : animations) {
        int32_t transformIndex = transAnim.GetTargetTransformIndex();
        if (transformIndex < 0) {
            continue;
        }
        auto trans = GetComponent<Transform>(handle, static_cast<uint32_t>(transformIndex));
        transAnim.Update(deltaTime, trans);
    }
}
