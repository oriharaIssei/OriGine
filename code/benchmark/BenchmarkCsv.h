#pragma once

/// stl
#include <string>
#include <vector>

/// engine
#include "benchmark/BenchmarkTypes.h"

namespace OriGine::Benchmark {

/// <summary>
/// ベンチマーク結果を4種のCSVファイル(サマリ / スコープ別集計 / フレーム時系列 / 呼び出し回数集計)へ出力する.
/// 出力先パスは _basePath の拡張子の手前に ".summary" / ".scopes" / ".frames" / ".counters" を
/// 挿入したものになる
/// (例: "out/run1.csv" -> "out/run1.summary.csv", "out/run1.scopes.csv", "out/run1.frames.csv", "out/run1.counters.csv").
/// 親ディレクトリが存在しない場合は作成する. 数値は小数点固定(%.4f相当)、ロケールに依存しない
/// 区切り文字("." 固定, 桁区切りなし)で出力する.
/// </summary>
/// <param name="_basePath">出力先のベースパス(--bench-csv で指定された値)</param>
/// <param name="_summary">サマリ情報</param>
/// <param name="_scopes">スコープ別集計結果</param>
/// <param name="_frames">フレーム時系列データ</param>
/// <param name="_counters">呼び出し回数集計結果(PROFILE_COUNT。Release構成やCallCounter無効ビルドでは空でよい)</param>
/// <returns>4ファイルすべて書き出せた場合はtrue</returns>
bool WriteBenchmarkCsv(
    const std::string& _basePath,
    const BenchmarkSummary& _summary,
    const std::vector<BenchmarkScopeStat>& _scopes,
    const std::vector<BenchmarkFrameRecord>& _frames,
    const std::vector<BenchmarkCounterStat>& _counters);

} // namespace OriGine::Benchmark
