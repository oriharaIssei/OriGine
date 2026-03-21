#include "ParticleSystemWorkSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/effect/particle/emitter/ParticleSystem.h"

using namespace OriGine;

/// <summary>
/// 初期化
/// </summary>
void ParticleSystemWorkSystem::Initialize() {
}

/// <summary>
/// 終了処理
/// </summary>
void ParticleSystemWorkSystem::Finalize() {
    entities_.clear();
}

/// <summary>
/// 各エンティティのエミッターを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void ParticleSystemWorkSystem::UpdateEntity(EntityHandle _handle) {
    const float deltaTime = Engine::GetInstance()->GetDeltaTime();

    auto& emitters = GetComponents<ParticleSystem>(_handle);

    if (emitters.empty()) {
        return;
    }

    for (auto& emitter : emitters) {
        if (!emitter.IsActive()) {
            continue;
        }

        // パーティクル更新
        for (auto& particle : emitter.particles_) {
            particle->Update(deltaTime);
        }
        std::erase_if(emitter.particles_, [](const std::shared_ptr<Particle>& particle) {
            return !particle->GetIsAlive();
        });

        // スポーン（タイミングは Emitter に委譲）
        int32_t spawnCount = emitter.emitter_.Update(deltaTime);
        if (spawnCount > 0) {
            emitter.SpawnParticle(spawnCount);
        }

        // パーティクル固有: 時間切れかつ全滅したら非アクティブ化
        if (emitter.emitter_.IsExpired() && emitter.particles_.empty()) {
            emitter.emitter_.Deactivate();
            continue;
        }

        // 描画用インスタンスデータを更新
        emitter.structuredTransform_.openData_.clear();
        for (auto& particle : emitter.particles_) {
            emitter.structuredTransform_.openData_.push_back(particle->GetTransform());
        }
    }
}
