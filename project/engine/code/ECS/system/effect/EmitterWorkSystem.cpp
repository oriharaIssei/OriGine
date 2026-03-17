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

    for (auto& emitter : emitters) {
        // アクティブでなければスキップ
        if (!emitter.isActive_) {
            continue;
        }

        // ループしない場合は残り時間を減らし、切れたら非アクティブ化
        if (!emitter.isLoop_) {
            emitter.leftActiveTime_ -= deltaTime;
            if (emitter.leftActiveTime_ <= 0.0f && emitter.particles_.empty()) {
                emitter.isActive_ = false;
                continue;
            }
        }

        // パーティクル更新
        for (auto& particle : emitter.particles_) {
            particle->Update(deltaTime);
        }
        std::erase_if(emitter.particles_, [](const std::shared_ptr<Particle>& particle) {
            return !particle->GetIsAlive();
        });

        // スポーン
        emitter.currentCoolTime_ -= deltaTime;
        if (emitter.leftActiveTime_ > 0.f) {
            if (emitter.currentCoolTime_ <= 0.0f) {
                emitter.currentCoolTime_ = emitter.spawnCoolTime_ / static_cast<float>(emitter.spawnParticleVal_);
                emitter.SpawnParticle((std::max)(1, static_cast<int32_t>(deltaTime / emitter.currentCoolTime_)));
            }
        }

        // 描画用インスタンスデータを更新
        emitter.structuredTransform_.openData_.clear();
        for (auto& particle : emitter.particles_) {
            emitter.structuredTransform_.openData_.push_back(particle->GetTransform());
        }
    }
}
