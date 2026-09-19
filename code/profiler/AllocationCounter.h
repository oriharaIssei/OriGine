#pragma once

/// stl
#include <array>
#include <cstddef>
#include <cstdint>

/// engine
#include "EngineConfig.h"

namespace OriGine::AllocationCounter {

/// <summary>
/// 1フレーム分のアロケーション統計
/// </summary>
struct FrameStats {
    uint64_t allocCount_ = 0; // このフレーム中に発生した確保回数
    uint64_t freeCount_  = 0; // このフレーム中に発生した解放回数
    uint64_t allocBytes_ = 0; // このフレーム中に確保した合計バイト数
    int64_t peakBytes_   = 0; // 計測開始からの確保中バイト数のピーク値(全フレーム共通の値をその都度記録)
};

/// <summary>
/// フレーム境界処理. 直前フレームからの差分を集計し、履歴リングバッファに積む.
/// OriGine::Profiler::BeginFrame() から1フレームに1回呼び出される.
/// Release構成ではoperator new/deleteの差し替えが行われないため、常に0が記録される.
/// </summary>
void OnFrameBegin();

/// <summary>
/// 直前フレームのアロケーション統計を取得する
/// </summary>
/// <returns>直前フレームの統計値</returns>
const FrameStats& GetLastFrameStats();

/// <summary>
/// 計測開始からの累積確保統計(フレーム境界に依存しない瞬時値)。
/// フレームループの外(保存・読み込みなど、OnFrameBeginを挟まない区間)のコストを測るには、
/// この値を区間の前後で読んで差分を取る(GetLastFrameStatsはOnFrameBeginを呼ぶフレームループ
/// 前提のため、フレームループを回さない計測には使えない)。
/// Release構成ではoperator new/deleteを差し替えないため常に0を返す。
/// </summary>
struct CumulativeStats {
    uint64_t allocCount_ = 0; // 計測開始からの総確保回数
    uint64_t allocBytes_ = 0; // 計測開始からの総確保バイト数(単調増加。解放では減算しない)
};

/// <summary>
/// 現在までの累積確保統計を取得する(OnFrameBeginの呼び出しとは無関係に、いつでも呼べる)
/// </summary>
/// <returns>現在の累積統計値</returns>
CumulativeStats GetCumulativeStats();

/// <summary>
/// 直近 kFrameHistorySize フレーム分の履歴を取得する(グラフ表示用のリングバッファ)
/// </summary>
/// <returns>履歴バッファへの参照</returns>
const std::array<FrameStats, OriGine::Config::Profiler::kFrameHistorySize>& GetHistory();

/// <summary>
/// 履歴バッファ内で最新のフレームが格納されているインデックス(ImGui::PlotLinesのvalues_offsetに使用)
/// </summary>
/// <returns>最新フレームのインデックス</returns>
size_t GetHistoryCursor();

} // namespace OriGine::AllocationCounter
