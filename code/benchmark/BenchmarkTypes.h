#pragma once

/// stl
#include <cstdint>
#include <string>

namespace OriGine::Benchmark {

/// <summary>
/// ベンチマーク実行中に固定するデルタタイム[秒](60fps相当)。
/// </summary>
/// <remarks>
/// 実時間のデルタタイムを使うと、マシンの実行速度やOSのスケジューリングジッターで
/// 1フレームの経過時間が回ごとに変わり、それに比例して移動量や衝突判定の回数まで
/// 変わってしまう(2026-09-17: 同一exeでフレーム時間13.3ms〜246msの幅が出て
/// 確保回数まで揺れた)。ベンチマークが計測したいのは「このシーンをこの負荷で
/// 動かした時のコスト」であり、負荷そのものが回ごとに揺れては前後比較ができない。
/// そのためベンチマーク実行中だけ RunBenchmarkLoop が DeltaTimer を固定モードにする。
/// </remarks>
constexpr float kFixedDeltaTimeSeconds = 1.0f / 60.0f;

/// <summary>
/// ベンチマークシーンの生成・実行パラメータ.
/// CLI(--bench-*)とエディタ(BenchmarkWindow)の両方から共通に使用する.
/// </summary>
struct BenchmarkConfig {
    uint32_t entityCount = 10000; // 生成するエンティティ数
    float extent          = 500.0f; // 配置する立方体の一辺(ワールド単位)。ブロードフェーズ負荷を左右する最重要パラメータ
    float radius           = 1.0f; // SphereColliderの半径
    uint32_t seed           = 12345; // 乱数シード(std::mt19937固定シード。決定性を担保する)
    uint32_t frames         = 600; // 計測フレーム数(CLIベンチのみ使用。到達したら自動終了する)
    uint32_t warmup         = 60; // 集計から除外する先頭フレーム数(CLIベンチのみ使用)
};

/// <summary>
/// 1フレーム分の計測結果(スパイク解析用の時系列データ)
/// </summary>
struct BenchmarkFrameRecord {
    uint32_t frameIndex_ = 0; // 0始まりのフレーム番号
    double frameMs_       = 0.0; // このフレームの所要時間(ミリ秒, 自前のwall-clock計測)
    uint64_t allocCount_  = 0; // このフレーム中の確保回数(Release構成では常に0)
    uint64_t allocBytes_  = 0; // このフレーム中の確保バイト数(Release構成では常に0)
};

/// <summary>
/// スコープ単位の集計結果(warmupを除いた対象フレームでの合計値。平均はCSV出力時にframeSampleCount_で除算する)
/// </summary>
struct BenchmarkScopeStat {
    std::string name_; // PROFILE_SCOPEに渡されたスコープ名
    uint64_t callCount_       = 0; // 対象フレーム全体での呼び出し回数の合計
    double totalMsSum_         = 0.0; // 対象フレーム全体での合計ms(子を含む)の総和
    double selfMsSum_          = 0.0; // 対象フレーム全体での自己ms(子を除く)の総和
    uint32_t frameSampleCount_ = 0; // 平均を取る際の母数(集計対象になったフレーム数)
};

/// <summary>
/// PROFILE_COUNTで数えた呼び出し回数の集計結果(warmupを除いた対象フレームでの合計値。
/// 平均はCSV出力時にframeSampleCount_で除算する。BenchmarkScopeStatの「回数だけ版」に相当する)
/// </summary>
struct BenchmarkCounterStat {
    std::string name_; // PROFILE_COUNTに渡されたカウンタ名(CallCounter::FrameStat::name_由来。切り詰められている場合がある)
    uint64_t totalCount_       = 0; // 対象フレーム全体での呼び出し回数の合計
    uint32_t frameSampleCount_ = 0; // 平均を取る際の母数(集計対象になったフレーム数)
};

/// <summary>
/// ベンチマークのサマリ結果(前後比較の起点になる代表値)
/// </summary>
struct BenchmarkSummary {
    BenchmarkConfig config_;
    float avgFrameMs_ = 0.0f;
    float p99FrameMs_ = 0.0f;
    float maxFrameMs_ = 0.0f;
    double avgAllocCountPerFrame_ = 0.0;
    double avgAllocBytesPerFrame_ = 0.0;
    uint32_t sampleFrameCount_    = 0; // 集計に使用したフレーム数(warmup除外後)
    // このベンチが固定デルタタイムで実行されたかどうかと、その値[秒]。
    // 後から見返したCSVが決定的な条件下で取られたものかを判別できるようにするための記録用フィールド
    // (計測結果そのものには使わない)
    bool fixedDeltaTimeUsed_    = false;
    float fixedDeltaTimeSeconds_ = 0.0f;
};

} // namespace OriGine::Benchmark
