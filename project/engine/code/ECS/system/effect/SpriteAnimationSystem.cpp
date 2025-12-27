#include "SpriteAnimationSystem.h"

/// engine
#define DELTA_TIME
#define ENGINE_ECS
#include "EngineInclude.h"
// component
#include "component/animation/SpriteAnimation.h"
#include "component/renderer/Sprite.h"

using namespace OriGine;

SpriteAnimationSystem::SpriteAnimationSystem() : ISystem(SystemCategory::Effect) {}
SpriteAnimationSystem::~SpriteAnimationSystem() {}

void SpriteAnimationSystem::Initialize() {}

void SpriteAnimationSystem::Finalize() {}

void SpriteAnimationSystem::UpdateEntity(EntityHandle _handle) {
    std::vector<SpriteAnimation>& spriteAnimations = GetComponents<SpriteAnimation>(_handle);
    std::vector<SpriteRenderer>& spriteRenderers   = GetComponents<SpriteRenderer>(_handle);

    if (spriteAnimations.empty() || spriteRenderers.empty()) {
        return; // コンポーネントが存在しない場合は何もしない
    }
    // DeltaTimeを取得
    const float deltaTime = GetMainDeltaTime();
    for (auto& spriteAnimation : spriteAnimations) {
        // 対応するSpriteRendererコンポーネントを取得
        SpriteRenderer* spriteRenderer = GetComponent<SpriteRenderer>(spriteAnimation.GetSpriteComponentHandle());
        if (!spriteRenderer) {
            // インデックスが範囲外の場合、またはコンポーネントが見つからない場合はスキップ
            continue;
        }
        // スプライトアニメーションの更新
        spriteAnimation.UpdateSpriteAnimation(deltaTime, spriteRenderer);
    }
}
