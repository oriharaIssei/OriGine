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

using namespace OriGine;

void PrimitiveNodeAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto* primitiveNodeAnimation = GetComponent<PrimitiveNodeAnimation>(_entity);
    if (primitiveNodeAnimation == nullptr) {
        return;
    }
    const float deltaTime = GetMainDeltaTime();

    PrimitiveMeshRendererBase* primitive = GetComponent<PlaneRenderer>(_entity);
    if (primitive == nullptr) {
        primitive = GetComponent<SphereRenderer>(_entity);
        if (primitive == nullptr) {
            primitive = GetComponent<RingRenderer>(_entity);
            if (primitive == nullptr) {
                primitive = GetComponent<BoxRenderer>(_entity);
                if (primitive == nullptr) {
                    return; // No primitive renderer found
                }
            }
        }
    }

    primitiveNodeAnimation->Update(deltaTime, &primitive->GetTransformBuff().openData_);
}
