#pragma once

/// stl
#include <cstdint>
#include <string>
#include <vector>

/// engine
#include "benchmark/BenchmarkTypes.h"

namespace OriGine::Benchmark {

/// <summary>
/// 保存・読み込み1回分の計測結果(D-2: 3D(シリアライズをディスクリプタ経由に書き換える段階)の
/// 前後比較用の器具。シリアライズ本体(SceneFactory / ComponentArray::Save|LoadComponents)は
/// このタスクの範囲外で、書き換えない)
/// </summary>
struct SerializeBenchRecord {
    uint32_t repeat_ = 0; // 1始まりの試行回数
    std::string case_;     // "save" または "load"
    double ms_       = 0.0; // 所要時間[ms](自前のwall-clock計測。frame_msと同じ考え方)
    uint64_t allocCount_ = 0; // この操作中の確保回数(Release構成では常に0)
    uint64_t allocBytes_ = 0; // この操作中の確保バイト数(Release構成では常に0)
    uint64_t jsonBytes_  = 0; // 保存したJSONのバイト数(インデント4。実ファイル書き出しの
                              // SceneJsonRegistry::SaveSceneと同じ形式に揃えている)
    uint32_t entityCount_ = 0; // シーンのエンティティ数(_config.entityCountと同じ)
};

/// <summary>
/// _config で指定したベンチマークシーン(BuildBenchmarkSceneと同じ生成規則。Transform +
/// Rigidbody + SphereCollider を持つエンティティを entityCount 個)を1つ構築し、
/// SceneFactory 経由の「保存」(CreateSceneJsonFromScene)と「読み込み」(BuildSceneFromJson)
/// だけを _repeat 回計測する。フレームループ(Update/Draw)は一切回さない
/// (計測フレーム内には保存・読み込みが無いため、通常の--benchとは別の経路として用意する)。
///
/// 保存は同じソースシーンを毎回読み取るだけ(CreateSceneJsonFromSceneは状態を変えない)なので、
/// _repeat回とも同じ結果になるはず(確保回数が決定的かどうかの検証対象そのもの)。
/// 読み込みは毎回まっさらなシーンへ読み込む。同じシーンへ繰り返し読み込むと2回目以降は
/// 既存Handleへの重複登録という1回目と違う経路を通ってしまい、
/// 「決定的なはずの確保回数」の検証にならなくなるため。
/// </summary>
/// <param name="_config">生成するシーンのパラメータ(entityCount / extent / radius / seed)。
/// frames / warmup は使わない(保存・読み込みはフレームループの外で計測するため)</param>
/// <param name="_repeat">save/loadそれぞれを繰り返す回数(既定5。1回目も含めて全件返す)</param>
/// <returns>_repeat×2件(save 1件 + load 1件を1セットとして_repeatセット)のレコード列</returns>
std::vector<SerializeBenchRecord> RunSerializeBenchmark(const BenchmarkConfig& _config, uint32_t _repeat);

} // namespace OriGine::Benchmark
