#pragma once

/// stl
#include <string>
#include <vector>

/// DLL境界
#include "OriGineApi.h"

namespace OriGine::Benchmark {

/// <summary>
/// main.cpp の ParseCommandLine() が返す引数リストに "--bench" または "--bench-serialize" が
/// 含まれている場合、対応するベンチマークを実行してCSVを出力したのち true を返す(呼び出し元は
/// これを合図にアプリケーションを即座に終了させること).
/// どちらも指定されていない場合は何もせず false を返す(既存の起動フローを一切変更しない).
/// 両方指定された場合は "--bench-serialize" を優先する(通常のフレーム計測は行わない).
///
/// 対応するCLI引数(通常のフレーム計測。"--bench" が必須):
///   --bench                    : ベンチマークモードを有効化する(これが無ければ他の引数は無視される)
///   --bench-entities=<uint>    : 生成するエンティティ数 (既定値 10000)
///   --bench-extent=<float>     : 配置する立方体の一辺 (既定値 500.0)
///   --bench-radius=<float>     : SphereColliderの半径 (既定値 1.0)
///   --bench-seed=<uint>        : 乱数シード (既定値 12345)
///   --bench-frames=<uint>      : 計測フレーム数 (既定値 600)
///   --bench-warmup=<uint>      : 集計から除外する先頭フレーム数 (既定値 60)
///   --bench-csv=<path>         : CSV出力先のベースパス (既定値 "./generated/benchmark/result.csv")
///
/// 対応するCLI引数(保存・読み込みだけの計測。Phase 3 D-2。"--bench-serialize" が必須):
///   --bench-serialize              : シリアライズベンチマークモードを有効化する
///   --bench-entities/-extent/-radius/-seed/-csv : 上と同じ意味(シーンの生成条件と出力先)
///   --bench-serialize-repeat=<uint> : save/loadをそれぞれ繰り返す回数 (既定値 5)
///   (--bench-frames / --bench-warmup はこちらでは使わない。フレームループを回さないため)
/// </summary>
/// <param name="_commandLines">main.cpp の ParseCommandLine() の戻り値</param>
/// <returns>いずれかのベンチマークを実行した場合は true</returns>
ORIGINE_API bool RunCliBenchmarkIfRequested(const std::vector<std::string>& _commandLines);

} // namespace OriGine::Benchmark
