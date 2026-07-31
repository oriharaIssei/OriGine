#pragma once

/// stl
#include <string>
#include <vector>

namespace OriGine::Benchmark {

/// <summary>
/// main.cpp の ParseCommandLine() が返す引数リストに "--bench" が含まれている場合、
/// 衝突判定負荷再現用のベンチマークシーンを構築し、指定フレーム数だけ実行してCSVを出力したのち
/// true を返す(呼び出し元はこれを合図にアプリケーションを即座に終了させること).
/// "--bench" が指定されていない場合は何もせず false を返す(既存の起動フローを一切変更しない).
///
/// 対応するCLI引数:
///   --bench                    : ベンチマークモードを有効化する(これが無ければ他の引数は無視される)
///   --bench-entities=<uint>    : 生成するエンティティ数 (既定値 10000)
///   --bench-extent=<float>     : 配置する立方体の一辺 (既定値 500.0)
///   --bench-radius=<float>     : SphereColliderの半径 (既定値 1.0)
///   --bench-seed=<uint>        : 乱数シード (既定値 12345)
///   --bench-frames=<uint>      : 計測フレーム数 (既定値 600)
///   --bench-warmup=<uint>      : 集計から除外する先頭フレーム数 (既定値 60)
///   --bench-csv=<path>         : CSV出力先のベースパス (既定値 "./generated/benchmark/result.csv")
/// </summary>
/// <param name="_commandLines">main.cpp の ParseCommandLine() の戻り値</param>
/// <returns>ベンチマークを実行した場合は true</returns>
bool RunCliBenchmarkIfRequested(const std::vector<std::string>& _commandLines);

} // namespace OriGine::Benchmark
