#pragma once

/// stl
#include <array>
#include <cstdint>
#include <vector>

/// engine
#include "EngineConfig.h"
#include "benchmark/BenchmarkTypes.h"

namespace OriGine::Benchmark {

/// <summary>
/// 直前フレームの生イベント列(Profiling::ProfileEventStream)を、動的確保を伴わずに走査して
/// スコープ別の呼び出し回数・合計ms・自己msを集計するアキュムレータ.
///
/// Profiler::BuildLastFrameTrees() はEditor表示専用に設計されたAPIで、呼び出すたびに
/// vector<ProfileThreadTree>やvector<ProfileTreeNode>を動的確保する。CLIベンチのように
/// 「毎フレーム」スコープ内訳を積算したい場合にこれをそのまま使うと、その確保自体が
/// AllocationCounterの数値を汚染し、計測対象(CollisionCheckSystem等)の数値を見誤らせてしまう。
/// そのためこのクラスは Profiling::ProfileTree.cpp と同じアルゴリズム(開始/終了イベントをスタックで
/// 対応付け、自己時間 = 合計時間 - 子の合計時間 として計算する)を、木を構築せず固定長テーブルへの
/// 直接加算に置き換えて実装している.
///
/// スコープ名の異なり数は実行時に増減しうるが、実運用上は起動直後の数フレームで出現し尽くして
/// 安定するため、kMaxScopeCount を超えない限りは以降の挿入(=動的確保)は発生しない.
/// </summary>
class ScopeAggregator {
public:
    static constexpr size_t kMaxScopeCount = 128; // 集計可能なスコープ名の異なり数の上限
    static constexpr size_t kMaxStackDepth = 64; // PROFILE_SCOPEのネスト深度の上限

    /// <summary>
    /// 登録されている全スレッドの「直前フレーム」のイベント列を集計に加算する.
    /// Profiler::BeginFrame() の直後(=ダブルバッファがスワップされ、直前フレームのイベント列が
    /// 確定した直後)に呼び出すこと.
    /// </summary>
    void AccumulateLastFrame();

    /// <summary>
    /// 集計結果を確定させて取得する(この呼び出しのみ動的確保を伴う。ループを抜けた後に1回だけ呼ぶこと).
    /// </summary>
    /// <returns>合計msの降順に並んだスコープ集計結果</returns>
    std::vector<BenchmarkScopeStat> BuildResult() const;

private:
    /// <summary>スコープ1つ分の累積値</summary>
    struct Entry {
        char name_[OriGine::Config::Profiler::kEventNameCapacity]{};
        bool used_          = false;
        uint64_t callCount_ = 0;
        double totalMsSum_  = 0.0;
        double selfMsSum_   = 0.0;
    };

    /// <summary>再帰呼び出し等のネストを追跡するためのスタックエントリ</summary>
    struct StackFrame {
        Entry* entry_        = nullptr;
        uint64_t startTicks_ = 0;
        double childAccumMs_ = 0.0;
    };

    /// <summary>
    /// 名前に対応するエントリを検索し、無ければ新規作成して返す(kMaxScopeCountを超える場合はnullptr).
    /// </summary>
    Entry* FindOrCreate(const char* _name);

    std::array<Entry, kMaxScopeCount> entries_{};
    size_t entryCount_      = 0;
    uint32_t countedFrames_ = 0; // 集計対象として加算したフレーム数(平均の母数)
};

} // namespace OriGine::Benchmark
