#include "MaterialAnimationWorkSystem.h"

/// engine
#define DELTA_TIME
#include "EngineInclude.h"

/// ECS
#include "component/animation/MaterialAnimation.h"
#include "component/material/Material.h"

void MaterialAnimationWorkSystem::UpdateEntity(GameEntity* _entity) {
    auto* materialAnimations = getComponents<MaterialAnimation>(_entity);
    if (materialAnimations == nullptr) {
        return;
    }
    const float deltaTime = getMainDeltaTime();

    for (auto& materialAnimation : *materialAnimations) {
        int32_t materialIndex = materialAnimation.getMaterialIndex();
        if (materialIndex < 0) {
            continue;
        }
        auto material = getComponent<Material>(_entity, static_cast<uint32_t>(materialIndex));
        materialAnimation.Update(deltaTime, material);
    }
}
