#pragma once

/// stl
#include <vector>

/// engine
#include "benchmark/BenchmarkTypes.h"

namespace OriGine {
class Scene;
class Engine;
}

namespace OriGine::Benchmark {

/// <summary>ベンチマーク実行結果一式</summary>
struct BenchmarkResult {
    BenchmarkSummary summary_;
    std::vector<BenchmarkFrameRecord> frames_;
    std::vector<BenchmarkScopeStat> scopes_;
};

/// <summary>
/// ベンチマークシーンを _config.frames フレームだけ実行し、フレームタイム・アロケーション・
/// スコープ別実行時間を計測する(CLIベンチマークの中核. 自動終了フローの一部として使う).
///
/// フレーム時間は Profiler の(Debug/Develop専用の)履歴に頼らず、このループ自身が
/// std::chrono::steady_clock で計測する(Release構成でも意味のある値を得るため、また
/// kFrameHistorySize=300 を超える長時間ベンチでも全フレーム分の時系列を残すため).
///
/// 計測ループ内での動的確保を避けるため、結果バッファは実行前に必要数だけ reserve する.
/// </summary>
/// <param name="_engine">エンジンインスタンス(BeginFrame等のフレーム制御に使用)</param>
/// <param name="_scene">計測対象のシーン(あらかじめ BuildBenchmarkScene で構築済みであること)</param>
/// <param name="_config">ベンチマークパラメータ</param>
/// <returns>計測結果</returns>
BenchmarkResult RunBenchmarkLoop(Engine* _engine, Scene* _scene, const BenchmarkConfig& _config);

} // namespace OriGine::Benchmark
