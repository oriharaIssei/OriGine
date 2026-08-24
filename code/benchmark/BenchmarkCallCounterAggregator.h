#pragma once

/// stl
#include <array>
#include <cstdint>
#include <vector>

/// engine
#include "EngineConfig.h"
#include "benchmark/BenchmarkTypes.h"
#include "profiler/CallCounter.h"

namespace OriGine::Benchmark {

/// <summary>
/// 直前フレームの CallCounter::GetLastFrameStats() を、動的確保を伴わずにフレームをまたいで
/// 積算するアキュムレータ。BenchmarkScopeAggregatorのカウンタ専用・簡略版に相当する。
///
/// ScopeAggregatorと違い、開始/終了イベントの対応付け(スタック)や名前の重複判定は不要である:
/// CallCounter::Register()が既に「同じ名前なら同じハンドル」を保証しているため、
/// GetLastFrameStats()が返す配列の添字はハンドルそのものであり、フレームをまたいで
/// 常に同じ計測点を指す(登録は追記のみで並び替え・削除が起きないため)。
/// そのためここでは名前を検索せず、返ってきた配列をそのままインデックスで加算するだけでよい。
///
/// 計測ループ内で毎フレーム呼ばれる想定のため、BuildResult()以外は一切動的確保を行わない
/// (理由はBenchmarkScopeAggregator.hの冒頭コメントを参照。ループ内の確保はavg_alloc_count_per_frame
/// を汚染し、計測結果そのものを信用できなくする)。
/// </summary>
class CallCounterAggregator {
public:
    /// <summary>
    /// 直前フレームの CallCounter::GetLastFrameStats() を加算する。
    /// Profiler::BeginFrame() の直後(=CallCounter::OnFrameBeginが直前フレーム分を確定させた直後)
    /// に呼び出すこと。
    /// </summary>
    void AccumulateLastFrame();

    /// <summary>
    /// 集計結果を確定させて取得する(この呼び出しのみ動的確保を伴う。ループを抜けた後に1回だけ呼ぶこと)。
    /// </summary>
    /// <returns>合計呼び出し回数の降順に並んだカウンタ集計結果</returns>
    std::vector<BenchmarkCounterStat> BuildResult() const;

private:
    /// <summary>カウンタ1つ分の累積値</summary>
    struct Entry {
        char name_[OriGine::Config::Profiler::kEventNameCapacity]{};
        bool used_           = false;
        uint64_t totalCount_ = 0;
    };

    // CallCounter::kMaxCountersと同じ上限にしておけば、CallCounter側で記録できている計測点を
    // ここで取りこぼすことはない(CallCounter自身がこれ以上増えないことを保証しているため)。
    std::array<Entry, OriGine::CallCounter::kMaxCounters> entries_{};
    size_t entryCount_      = 0;
    uint32_t countedFrames_ = 0; // 集計対象として加算したフレーム数(平均の母数)
};

} // namespace OriGine::Benchmark
