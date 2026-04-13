#include "MaterialAnimationWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
#include "component/animation/MaterialAnimation.h"
#include "component/material/Material.h"

using namespace OriGine;

/// <summary>
/// 各エンティティのマテリアルアニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void MaterialAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto& materialAnimations = GetComponents<MaterialAnimation>(_handle);
    if (materialAnimations.empty()) {
        return;
    }
    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    for (auto& materialAnimation : materialAnimations) {
        int32_t materialIndex = materialAnimation.GetMaterialIndex();
        if (materialIndex < 0) {
            continue;
        }
        auto material = GetComponent<Material>(_handle, static_cast<uint32_t>(materialIndex));
        materialAnimation.Update(deltaTime, material);
    }
}
