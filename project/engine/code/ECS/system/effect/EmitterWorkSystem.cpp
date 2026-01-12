#include "EmitterWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/effect/particle/emitter/Emitter.h"

using namespace OriGine;

/// <summary>
/// 初期化
/// </summary>
void EmitterWorkSystem::Initialize() {
}

/// <summary>
/// 終了処理
/// </summary>
void EmitterWorkSystem::Finalize() {
    entities_.clear();
}

/// <summary>
/// 各エンティティのエミッターを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void EmitterWorkSystem::UpdateEntity(EntityHandle _handle) {
    const float deltaTime = Engine::GetInstance()->GetDeltaTime();

    auto& emitters = GetComponents<Emitter>(_handle);

    if (emitters.empty()) {
        return;
    }

    for (auto& comp : emitters) {
        comp.Update(deltaTime);
    }
}
