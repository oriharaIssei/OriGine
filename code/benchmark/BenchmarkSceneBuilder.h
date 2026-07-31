#pragma once

#include "benchmark/BenchmarkTypes.h"

namespace OriGine {
class Scene;
}

namespace OriGine::Benchmark {

/// <summary>
/// 衝突判定負荷再現用のベンチマークシーンを構築する.
/// Transform + Rigidbody + SphereCollider を持つエンティティを _config.entityCount 個生成し、
/// MoveSystemByRigidBody / CollisionCheckSystem に登録する.
/// 同じ _config.seed であれば何度実行しても全く同じ配置になる(std::mt19937を固定シードで使用し、
/// std::random_device や時刻を混ぜない)ことで、最適化前後の比較に使えるようにしている.
/// </summary>
/// <param name="_scene">構築先のシーン(あらかじめ Scene::Initialize() 済みであること)</param>
/// <param name="_config">ベンチマークパラメータ</param>
void BuildBenchmarkScene(Scene* _scene, const BenchmarkConfig& _config);

} // namespace OriGine::Benchmark
