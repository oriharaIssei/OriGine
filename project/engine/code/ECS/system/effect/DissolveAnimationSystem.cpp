#include "DissolveAnimationSystem.h"

/// engine
#include "Engine.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
DissolveAnimationSystem::DissolveAnimationSystem() : ISystem(SystemCategory::Effect) {}

/// <summary>
/// デストラクタ
/// </summary>
OriGine::DissolveAnimationSystem::~DissolveAnimationSystem() {}

/// <summary>
/// 初期化
/// </summary>
void OriGine::DissolveAnimationSystem::Initialize() {
    Engine::GetInstance()->GetDeltaTimer()->SetTimeScale("Effect", 1.0f);
}

/// <summary>
/// 終了処理
/// </summary>
void OriGine::DissolveAnimationSystem::Finalize() {}

/// <summary>
/// エンティティのディゾルブアニメーションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void OriGine::DissolveAnimationSystem::UpdateEntity(EntityHandle _handle) {
    auto* dissolveAnimComp  = GetComponent<DissolveAnimation>(_handle);
    auto* dissolveParamComp = GetComponent<DissolveEffectParam>(_handle);

    if (dissolveAnimComp == nullptr || dissolveParamComp == nullptr) {
        return;
    }

    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");
    dissolveAnimComp->Update(deltaTime, dissolveParamComp);
}
