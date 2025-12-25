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
    auto& animations = GetComponents<TransformAnimation>(_handle);
    if (animations.empty()) {
        return;
    }

    const float deltaTime = GetMainDeltaTime();

    for (auto& transAnim : animations) {
        int32_t transformIndex = transAnim.GetTargetTransformIndex();
        if (transformIndex < 0) {
            continue;
        }
        auto trans = GetComponent<Transform>(_handle, static_cast<uint32_t>(transformIndex));
        transAnim.Update(deltaTime, trans);
    }
}
