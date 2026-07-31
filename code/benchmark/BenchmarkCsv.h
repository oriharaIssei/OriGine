#pragma once

/// stl
#include <string>
#include <vector>

/// engine
#include "benchmark/BenchmarkTypes.h"

namespace OriGine::Benchmark {

/// <summary>
/// ベンチマーク結果を3種のCSVファイル(サマリ / スコープ別集計 / フレーム時系列)へ出力する.
/// 出力先パスは _basePath の拡張子の手前に ".summary" / ".scopes" / ".frames" を挿入したものになる
/// (例: "out/run1.csv" -> "out/run1.summary.csv", "out/run1.scopes.csv", "out/run1.frames.csv").
/// 親ディレクトリが存在しない場合は作成する. 数値は小数点固定(%.4f相当)、ロケールに依存しない
/// 区切り文字("." 固定, 桁区切りなし)で出力する.
/// </summary>
/// <param name="_basePath">出力先のベースパス(--bench-csv で指定された値)</param>
/// <param name="_summary">サマリ情報</param>
/// <param name="_scopes">スコープ別集計結果</param>
/// <param name="_frames">フレーム時系列データ</param>
/// <returns>3ファイルすべて書き出せた場合はtrue</returns>
bool WriteBenchmarkCsv(
    const std::string& _basePath,
    const BenchmarkSummary& _summary,
    const std::vector<BenchmarkScopeStat>& _scopes,
    const std::vector<BenchmarkFrameRecord>& _frames);

} // namespace OriGine::Benchmark
