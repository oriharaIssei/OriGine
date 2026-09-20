#include "AllocationCounter.h"

/// stl
#include <atomic>

// ============================================================================
// このファイルは「アロケーション統計の状態とアクセサ」だけを持つ(Phase 4 4D D-4)。
// OriGine.dll に1つだけリンクされ、プロセス全体で共有する状態(atomicなカウンタ)を保持する。
//
// operator new/delete の差し替え本体は AllocationCounterHook.cpp に分離してある。
// グローバル operator new/delete の差し替えは「モジュール単位」でしか効かない
// ―― DLL の中で operator new を差し替えても、EXE 側が行う確保は EXE 自身にリンクされた
// operator new/delete(既定では ucrtbase.dll のもの)が使われ、数えられない。
// そのため差し替え本体は各モジュール(OriGine.dll / ECS_TestGame.exe / ECS_TestEditor.exe)に
// それぞれ配置し、ここの RecordAlloc/RecordFree(ORIGINE_API でエクスポート)を呼んで
// 状態を1箇所に集約させる設計にした。分けずに DLL 側だけに差し替えを置くと、
// 4D の after で確保回数が(EXE側の分だけ)静かに減る ―― 「速くなった」のではなく
// 「壊れた」計測になる(6章の罠1と同じ)。
// ============================================================================

namespace {

// --------------------------------------------------------------------------
// 常時保持する統計(アトミック)。
// 匿名名前空間はこの翻訳単位からしか非修飾名で参照できないが、OriGine::AllocationCounter
// 側の実装は同じ翻訳単位にあるので直接参照できる。他モジュール(EXE側)の
// AllocationCounterHook.cpp からは RecordAlloc/RecordFree 経由でのみ触れる。
// --------------------------------------------------------------------------
std::atomic<uint64_t> g_totalAllocCount{0}; // 計測開始からの総確保回数
std::atomic<uint64_t> g_totalFreeCount{0}; // 計測開始からの総解放回数
std::atomic<int64_t> g_totalAllocBytesAccum{0}; // 確保された合計バイト数(単調増加。フリーでは減算しない)
std::atomic<int64_t> g_currentBytes{0}; // 現在確保中のバイト数
std::atomic<int64_t> g_peakBytes{0}; // 計測開始からの確保中バイト数のピーク値

// 直前フレーム終了時点までの累積値(フレーム差分計算用。Profiler::BeginFrameを呼ぶ
// スレッドからのみ触るため非アトミックで良い)
uint64_t g_prevAllocCount      = 0;
uint64_t g_prevFreeCount       = 0;
int64_t g_prevAllocBytesAccum  = 0;

OriGine::AllocationCounter::FrameStats g_lastFrameStats{};
std::array<OriGine::AllocationCounter::FrameStats, OriGine::Config::Profiler::kFrameHistorySize> g_history{};
size_t g_historyCursor = 0;

/// <summary>
/// ピーク使用量をアトミックに更新する (CAS loop)
/// </summary>
void UpdatePeak(int64_t _current) {
    int64_t peak = g_peakBytes.load(std::memory_order_relaxed);
    while (_current > peak && !g_peakBytes.compare_exchange_weak(peak, _current, std::memory_order_relaxed)) {
        // ループ内で peak が最新値に更新される
    }
}

} // namespace

namespace OriGine::AllocationCounter {

void RecordAlloc(size_t _size) noexcept {
    g_totalAllocCount.fetch_add(1, std::memory_order_relaxed);
    g_totalAllocBytesAccum.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed);
    const int64_t current = g_currentBytes.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed) + static_cast<int64_t>(_size);
    UpdatePeak(current);
}

void RecordFree(size_t _size) noexcept {
    g_totalFreeCount.fetch_add(1, std::memory_order_relaxed);
    g_currentBytes.fetch_sub(static_cast<int64_t>(_size), std::memory_order_relaxed);
}

void OnFrameBegin() {
    const uint64_t allocCount   = g_totalAllocCount.load(std::memory_order_relaxed);
    const uint64_t freeCount    = g_totalFreeCount.load(std::memory_order_relaxed);
    const int64_t allocBytesAcc = g_totalAllocBytesAccum.load(std::memory_order_relaxed);

    FrameStats stats;
    stats.allocCount_ = allocCount - g_prevAllocCount;
    stats.freeCount_  = freeCount - g_prevFreeCount;
    stats.allocBytes_ = static_cast<uint64_t>(allocBytesAcc - g_prevAllocBytesAccum);
    stats.peakBytes_  = g_peakBytes.load(std::memory_order_relaxed);

    g_prevAllocCount     = allocCount;
    g_prevFreeCount      = freeCount;
    g_prevAllocBytesAccum = allocBytesAcc;

    g_lastFrameStats = stats;

    g_historyCursor            = (g_historyCursor + 1) % OriGine::Config::Profiler::kFrameHistorySize;
    g_history[g_historyCursor] = stats;
}

const FrameStats& GetLastFrameStats() {
    return g_lastFrameStats;
}

CumulativeStats GetCumulativeStats() {
    CumulativeStats stats;
    stats.allocCount_ = g_totalAllocCount.load(std::memory_order_relaxed);
    stats.allocBytes_ = static_cast<uint64_t>(g_totalAllocBytesAccum.load(std::memory_order_relaxed));
    return stats;
}

const std::array<FrameStats, OriGine::Config::Profiler::kFrameHistorySize>& GetHistory() {
    return g_history;
}

size_t GetHistoryCursor() {
    return g_historyCursor;
}

} // namespace OriGine::AllocationCounter
