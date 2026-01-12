#include "TransformAnimationWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/animation/TransformAnimation.h"
#include "component/transform/Transform.h"

using namespace OriGine;

/// <summary>
/// 各エンティティのトランスフォームアニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void TransformAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto& animations = GetComponents<TransformAnimation>(_handle);
    if (animations.empty()) {
        return;
    }

    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    for (auto& transAnim : animations) {
        int32_t transformIndex = transAnim.GetTargetTransformIndex();
        if (transformIndex < 0) {
            continue;
        }
        auto trans = GetComponent<Transform>(_handle, static_cast<uint32_t>(transformIndex));
        transAnim.Update(deltaTime, trans);
    }
}
