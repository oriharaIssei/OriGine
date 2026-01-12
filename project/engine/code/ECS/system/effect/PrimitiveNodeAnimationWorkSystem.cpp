#include "PrimitiveNodeAnimationWorkSystem.h"

/// engine
#include "Engine.h"
#define ENGINE_ECS
#include "EngineInclude.h"
// component
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

using namespace OriGine;

/// <summary>
/// 各エンティティのプリミティブノードアニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void PrimitiveNodeAnimationWorkSystem::UpdateEntity(EntityHandle _handle) {
    auto* primitiveNodeAnimation = GetComponent<PrimitiveNodeAnimation>(_handle);
    if (primitiveNodeAnimation == nullptr) {
        return;
    }
    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    PrimitiveMeshRendererBase* primitive = GetComponent<PlaneRenderer>(_handle);
    if (primitive == nullptr) {
        primitive = GetComponent<SphereRenderer>(_handle);
        if (primitive == nullptr) {
            primitive = GetComponent<RingRenderer>(_handle);
            if (primitive == nullptr) {
                primitive = GetComponent<BoxRenderer>(_handle);
                if (primitive == nullptr) {
                    return; // No primitive renderer found
                }
            }
        }
    }

    primitiveNodeAnimation->Update(deltaTime, &primitive->GetTransformBuff().openData_);
}
