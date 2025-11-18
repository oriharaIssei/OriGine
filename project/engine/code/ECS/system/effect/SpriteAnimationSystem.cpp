#include "SpriteAnimationSystem.h"

/// engine
#define DELTA_TIME
#define ENGINE_ECS
#include "EngineInclude.h"
// component
#include "component/animation/SpriteAnimation.h"
#include "component/renderer/Sprite.h"

SpriteAnimationSystem::SpriteAnimationSystem() : ISystem(SystemCategory::Effect) {}

SpriteAnimationSystem::~SpriteAnimationSystem() {}

void SpriteAnimationSystem::Initialize() {}

void SpriteAnimationSystem::Finalize() {}

void SpriteAnimationSystem::UpdateEntity(Entity* _entity) {
    auto spriteAnimations          = GetComponents<SpriteAnimation>(_entity);
    SpriteRenderer* spriteRenderer = GetComponent<SpriteRenderer>(_entity);

    if (!spriteAnimations || !spriteRenderer) {
        return; // コンポーネントが存在しない場合は何もしない
    }
    // DeltaTimeを取得
    const float deltaTime = GetMainDeltaTime();
    for (auto& spriteAnimation : *spriteAnimations) {
        // スプライトアニメーションの更新
        spriteAnimation.UpdateSpriteAnimation(deltaTime, spriteRenderer);
    }
}
