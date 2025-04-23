#include "PrimitiveNodeAnimationWorkSystem.h"

/// engine
#define ENGINE_ECS
#define DELTA_TIME
#include "EngineInclude.h"
// component
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/renderer/primitive/Primitive.h"

void PrimitiveNodeAnimationWorkSystem::UpdateEntity(GameEntity* _entity) {
    auto* primitiveNodeAnimation = getComponent<PrimitiveNodeAnimation>(_entity);
    if (primitiveNodeAnimation == nullptr) {
        return;
    }
    auto* primitive = getComponent<PlaneRenderer>(_entity);
    if (primitive == nullptr) {
        return;
    }
    const float deltaTime = getMainDeltaTime();
    
    primitiveNodeAnimation->Update(deltaTime, &primitive->getTransformBuff().openData_, &primitive->getMaterialBuff().openData_);
    primitive->getMaterialBuff().ConvertToBuffer();
}
