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

void EffectAutoDestroySystem::UpdateEntity(OriGine::Entity* _entity) {
    bool isAlive = false;

    auto materialAnimations = GetComponents<MaterialAnimation>(_entity);
    if (materialAnimations) {
        for (auto& anim : *materialAnimations) {
            isAlive |= anim.GetAnimationIsPlay();
        }
    }
    auto modelAnimations = GetComponents<ModelNodeAnimation>(_entity);
    if (modelAnimations) {
        for (auto& anim : *modelAnimations) {
            isAlive |= anim.IsPlay();
        }
    }
    auto primAnimations = GetComponents<PrimitiveNodeAnimation>(_entity);
    if (primAnimations) {
        for (auto& anim : *primAnimations) {
            isAlive |= anim.GetAnimationIsPlay();
        }
    }
    auto skinningAnimations = GetComponents<SkinningAnimationComponent>(_entity);
    if (skinningAnimations) {
        for (auto& anim : *skinningAnimations) {
            isAlive |= anim.IsPlay();
        }
    }
    auto spriteAnimations = GetComponents<SpriteAnimation>(_entity);
    if (spriteAnimations) {
        for (auto& anim : *spriteAnimations) {
            isAlive |= anim.IsPlaying();
        }
    }
    auto transAnimations = GetComponents<TransformAnimation>(_entity);
    if (transAnimations) {
        for (auto& anim : *transAnimations) {
            isAlive |= anim.IsPlaying();
        }
    }

    if (!isAlive) {
        GetScene()->AddDeleteEntity(_entity->GetID());
    }
}
