#include "MaterialAnimationWorkSystem.h"

/// engine
#define DELTA_TIME
#include "EngineInclude.h"

/// ECS
#include "component/animation/MaterialAnimation.h"
#include "component/material/Material.h"

using namespace OriGine;

void MaterialAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto& materialAnimations = GetComponents<MaterialAnimation>(_handle);
    if (materialAnimations.empty()) {
        return;
    }
    const float deltaTime = GetMainDeltaTime();

    for (auto& materialAnimation : materialAnimations) {
        int32_t materialIndex = materialAnimation.GetMaterialIndex();
        if (materialIndex < 0) {
            continue;
        }
        auto material = GetComponent<Material>(_handle, static_cast<uint32_t>(materialIndex));
        materialAnimation.Update(deltaTime, material);
    }
}
