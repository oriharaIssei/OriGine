#include "PrimitiveNodeAnimationWorkSystem.h"

/// engine
#define ENGINE_ECS
#define DELTA_TIME
#include "EngineInclude.h"
// component
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

void PrimitiveNodeAnimationWorkSystem::UpdateEntity(Entity* _entity) {
    auto* primitiveNodeAnimation = getComponent<PrimitiveNodeAnimation>(_entity);
    if (primitiveNodeAnimation == nullptr) {
        return;
    }
    const float deltaTime = getMainDeltaTime();

    PrimitiveMeshRendererBase* primitive = getComponent<PlaneRenderer>(_entity);
    if (primitive == nullptr) {
        primitive = getComponent<SphereRenderer>(_entity);
        if (primitive == nullptr) {
            primitive = getComponent<RingRenderer>(_entity);
            if (primitive == nullptr) {
                primitive = getComponent<BoxRenderer>(_entity);
                if (primitive == nullptr) {
                    return; // No primitive renderer found
                }
            }
        }
    }

    primitiveNodeAnimation->Update(deltaTime, &primitive->getTransformBuff().openData_);
}
