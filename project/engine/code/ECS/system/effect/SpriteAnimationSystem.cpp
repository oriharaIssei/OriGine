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
    int32_t maxIndex = static_cast<int32_t>(spriteRenderers.size()) - 1;
    // DeltaTimeを取得
    const float deltaTime = GetMainDeltaTime();
    for (auto& spriteAnimation : spriteAnimations) {
        // 対応するSpriteRendererコンポーネントを取得
        int32_t spriteIndex = spriteAnimation.GetSpriteComponentIndex();
        if (spriteIndex < 0 || spriteIndex > maxIndex) {
            continue; // 無効なインデックスの場合はスキップ
        }

        SpriteRenderer* spriteRenderer = &(spriteRenderers)[spriteIndex];
        // スプライトアニメーションの更新
        spriteAnimation.UpdateSpriteAnimation(deltaTime, spriteRenderer);
    }
}
