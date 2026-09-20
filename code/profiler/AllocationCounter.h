#pragma once

/// stl
#include <array>
#include <cstddef>
#include <cstdint>

/// engine
#include "EngineConfig.h"

/// DLL境界
#include "OriGineApi.h"

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
ORIGINE_API void OnFrameBegin();

/// <summary>
/// 直前フレームのアロケーション統計を取得する
/// </summary>
/// <returns>直前フレームの統計値</returns>
ORIGINE_API const FrameStats& GetLastFrameStats();

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
ORIGINE_API CumulativeStats GetCumulativeStats();

/// <summary>
/// 直近 kFrameHistorySize フレーム分の履歴を取得する(グラフ表示用のリングバッファ)
/// </summary>
/// <returns>履歴バッファへの参照</returns>
ORIGINE_API const std::array<FrameStats, OriGine::Config::Profiler::kFrameHistorySize>& GetHistory();

/// <summary>
/// 履歴バッファ内で最新のフレームが格納されているインデックス(ImGui::PlotLinesのvalues_offsetに使用)
/// </summary>
/// <returns>最新フレームのインデックス</returns>
ORIGINE_API size_t GetHistoryCursor();

// ============================================================================
// operator new/delete の差し替え本体(AllocationCounterHook.cpp)から呼ばれるアクセサ(4D D-4)。
// ============================================================================
//
// なぜ分けるのか: グローバル operator new/delete の差し替えは「モジュール単位」でしか効かない
// (DLLの中でoperator newを差し替えても、EXE側が行う確保はEXE自身にリンクされた
// operator new/delete ―― ucrtbase.dll 既定のもの ―― が使われ、数えられない)。
// 一方で「確保回数・バイト数」という状態そのものは OriGine.dll に1つだけ存在してほしい
// (プロセス全体の合計を Profiler/ProfilerWindow が読むため)。
// そのため「状態+集計(このファイル、OriGine.dll に1つ)」と
// 「operator new/delete の差し替え本体(AllocationCounterHook.cpp、DLLと各EXEの計3箇所に配置)」
// を分離し、後者はこの2関数を呼ぶだけにする。
//
/// <summary>
/// 1回分の確保を記録する(総確保回数・総確保バイト数・現在確保中バイト数・ピーク値の更新)。
/// AllocationCounterHook.cpp の TrackedAllocate から、確保が成功するたびに呼ばれる。
/// </summary>
/// <param name="_size">要求バイト数(ヘッダ・アライメント調整分を含まないユーザー要求分)</param>
ORIGINE_API void RecordAlloc(size_t _size) noexcept;

/// <summary>
/// 1回分の解放を記録する(総解放回数・現在確保中バイト数の更新)。
/// AllocationCounterHook.cpp の TrackedFree から、解放のたびに呼ばれる。
/// </summary>
/// <param name="_size">TrackedAllocate時にヘッダへ記録しておいた元の要求バイト数</param>
ORIGINE_API void RecordFree(size_t _size) noexcept;

} // namespace OriGine::AllocationCounter
