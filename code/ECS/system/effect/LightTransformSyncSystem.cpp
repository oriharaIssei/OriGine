#include "LightTransformSyncSystem.h"

/// ECS
// component
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/transform/Transform.h"

using namespace OriGine;

void LightTransformSyncSystem::UpdateEntity(EntityHandle _handle) {
    // PointLight の位置を Transform から同期
    {
        auto& pointLights = GetComponents<PointLight>(_handle);
        for (auto& light : pointLights) {
            if (light.targetTransformIndex < 0) {
                continue;
            }
            auto* transform = GetComponent<Transform>(_handle, static_cast<uint32_t>(light.targetTransformIndex));
            if (!transform) {
                continue;
            }
            light.pos = transform->GetWorldTranslate();
        }
    }

    // SpotLight の位置・方向を Transform から同期
    {
        auto& spotLights = GetComponents<SpotLight>(_handle);
        for (auto& light : spotLights) {
            if (light.targetTransformIndex < 0) {
                continue;
            }
            auto* transform = GetComponent<Transform>(_handle, static_cast<uint32_t>(light.targetTransformIndex));
            if (!transform) {
                continue;
            }
            light.pos       = transform->GetWorldTranslate();
            light.direction = Vec3f::Normalize(transform->FrontVector());
        }
    }
}
