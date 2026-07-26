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
#include "component/effect/CameraAction.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ。
/// エンティティの生存/破棄を決める処理なので、StateTransition カテゴリに属する
/// （描画やエフェクト更新がすべて終わった後に実行させるため）
/// </summary>
EffectAutoDestroySystem::EffectAutoDestroySystem() : ISystem(SystemCategory::StateTransition) {}

/// <summary>
/// デストラクタ
/// </summary>
EffectAutoDestroySystem::~EffectAutoDestroySystem() {}

/// <summary>
/// 初期化処理。保持する状態が無いため何もしない
/// </summary>
void EffectAutoDestroySystem::Initialize() {}

/// <summary>
/// 終了処理。保持する状態が無いため何もしない
/// </summary>
void EffectAutoDestroySystem::Finalize() {}

/// <summary>
/// 再生中のアニメーションが1つも残っていないエンティティを、破棄対象としてシーンに登録する。
/// 使い捨てのエフェクト用エンティティを、再生完了後に自動で片付けるためのシステム
/// </summary>
/// <param name="_handle">判定対象のエンティティハンドル</param>
void EffectAutoDestroySystem::UpdateEntity(const OriGine::EntityHandle& _handle) {
    // 対象エンティティが持ちうる各種アニメーション/カメラアクションのいずれかが
    // 再生中であれば、まだ生存させる(いずれも再生中でなければ破棄対象とする)
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

    auto& cameraActions = GetComponents<CameraAction>(_handle);
    for (auto& action : cameraActions) {
        isAlive |= action.isPlaying();
    }

    // 全アニメーション/アクションが終了していれば、シーンに削除を予約する
    if (!isAlive) {
        GetScene()->AddDeleteEntity(_handle);
    }
}
