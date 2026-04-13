#include "TransformRateAnimationWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/animation/TransformRateAnimation.h"
#include "component/transform/Transform.h"

using namespace OriGine;

/// <summary>
/// 各エンティティの速度・加速度アニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void TransformRateAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto& animations = GetComponents<TransformRateAnimation>(_handle);
    if (animations.empty()) {
        return;
    }

    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    for (auto& rateAnim : animations) {
        int32_t transformIndex = rateAnim.GetTargetTransformIndex();
        if (transformIndex < 0) {
            continue;
        }
        auto trans = GetComponent<Transform>(_handle, static_cast<uint32_t>(transformIndex));
        rateAnim.Update(deltaTime, trans);
    }
}
