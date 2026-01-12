#include "SpriteAnimationSystem.h"

/// engine
#include "Engine.h"
#define ENGINE_ECS
#include "EngineInclude.h"
// component
#include "component/animation/SpriteAnimation.h"
#include "component/renderer/Sprite.h"

using namespace OriGine;

SpriteAnimationSystem::SpriteAnimationSystem() : ISystem(SystemCategory::Effect) {}

/// <summary>
/// デストラクタ
/// </summary>
SpriteAnimationSystem::~SpriteAnimationSystem() {}

/// <summary>
/// 初期化処理
/// </summary>
void SpriteAnimationSystem::Initialize() {}

/// <summary>
/// 終了処理
/// </summary>
void SpriteAnimationSystem::Finalize() {}

/// <summary>
/// 各エンティティのスプライトアニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void SpriteAnimationSystem::UpdateEntity(EntityHandle _handle) {
    std::vector<SpriteAnimation>& spriteAnimations = GetComponents<SpriteAnimation>(_handle);
    std::vector<SpriteRenderer>& spriteRenderers   = GetComponents<SpriteRenderer>(_handle);

    if (spriteAnimations.empty() || spriteRenderers.empty()) {
        return; // コンポーネントが存在しない場合は何もしない
    }
    // DeltaTimerを取得
    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");
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
