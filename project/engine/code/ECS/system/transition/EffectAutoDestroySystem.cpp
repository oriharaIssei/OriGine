#include "EffectAutoDestroySystem.h"

/// engine
#include "scene/Scene.h"

/// ECS
// component
#include "component/animation/MaterialAnimation.h"
#include "component/animation/ModelNodeAnimation.h"
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/animation/SkinningAnimationComponent.h"
#include "component/animation/SpriteAnimation.h"
#include "component/animation/TransformAnimation.h"

using namespace OriGine;

EffectAutoDestroySystem::EffectAutoDestroySystem() : ISystem(SystemCategory::StateTransition) {}
EffectAutoDestroySystem::~EffectAutoDestroySystem() {}

void EffectAutoDestroySystem::Initialize() {}
void EffectAutoDestroySystem::Finalize() {}

void EffectAutoDestroySystem::UpdateEntity(OriGine::EntityHandle _handle) {
    bool isAlive = false;

    auto& materialAnimations = GetComponents<MaterialAnimation>(_handle);
    for (auto& anim : materialAnimations) {
        isAlive |= anim.GetAnimationIsPlay();
    }

    auto& modelAnimations = GetComponents<ModelNodeAnimation>(_handle);
    for (auto& anim : modelAnimations) {
        isAlive |= anim.IsPlay();
    }

    auto& primAnimations = GetComponents<PrimitiveNodeAnimation>(_handle);
    for (auto& anim : primAnimations) {
        isAlive |= anim.GetAnimationIsPlay();
    }

    auto& skinningAnimations = GetComponents<SkinningAnimationComponent>(_handle);
    for (auto& anim : skinningAnimations) {
        isAlive |= anim.IsPlay();
    }

    auto& spriteAnimations = GetComponents<SpriteAnimation>(_handle);
    for (auto& anim : spriteAnimations) {
        isAlive |= anim.IsPlaying();
    }

    auto& transAnimations = GetComponents<TransformAnimation>(_handle);
    for (auto& anim : transAnimations) {
        isAlive |= anim.IsPlaying();
    }

    if (!isAlive) {
        GetScene()->AddDeleteEntity(_handle);
    }
}
