#include "AllocationCounter.h"

/// stl
#include <atomic>
#include <cstdlib>
#include <new>

// ============================================================================
// このファイルはアロケーションカウンタの実装であり、グローバル operator new/delete の
// 差し替えを含む「唯一の翻訳単位」である(要件: 差し替えは1つの翻訳単位に閉じること)。
//
// 差し替え本体(下部の operator new/delete 群)は `#if !defined(_RELEASE)` で
// 丸ごとガードされている。Release構成ではこのファイル内に operator new/delete の
// 定義が一切現れないため、リンク時にCRT標準の operator new/delete がそのまま
// 採用される。つまり「リンクで有効化する」とは、Debug/Developでは本ファイルの
// オブジェクトが定義するシンボルが標準ライブラリのものより優先してリンクされ、
// Releaseでは本ファイルがシンボルを提供しないためリンクされない、という意味である。
// ============================================================================

namespace {

// --------------------------------------------------------------------------
// 常時保持する統計(アトミック)。
// 匿名名前空間はファイル全体から非修飾名で参照できるため、
// OriGine::AllocationCounter 側の実装と下部の operator new/delete 側の
// 両方から同じ実体を参照できる。
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

const std::array<FrameStats, OriGine::Config::Profiler::kFrameHistorySize>& GetHistory() {
    return g_history;
}

size_t GetHistoryCursor() {
    return g_historyCursor;
}

} // namespace OriGine::AllocationCounter

// ============================================================================
// operator new / delete のグローバル差し替え (Debug / Develop構成のみ)
// ============================================================================
#if !defined(_RELEASE)

namespace {

/// <summary>
/// 確保したブロックの直前に埋め込むヘッダ.
/// sized/unsizedどちらのdelete呼び出しでも正確なサイズと解放先を得られるようにするため、
/// コンパイラのsized delete対応有無に依存せず常にこのヘッダを頼りに解放する.
/// </summary>
struct AllocHeader {
    size_t size_;   // ユーザーが要求したバイト数
    size_t offset_; // malloc の生ポインタからユーザーポインタまでのオフセット(free時に生ポインタへ戻すため)
};

constexpr size_t kMinAlignment = alignof(std::max_align_t);

/// <summary>
/// 追跡付きのメモリ確保. 要求バイト数 + ヘッダ + アライメント調整分を malloc で確保し、
/// ユーザーへ返すポインタの直前にヘッダを埋め込む.
/// </summary>
void* TrackedAllocate(size_t _size, size_t _align, bool _noThrow) {
    if (_align < kMinAlignment) {
        _align = kMinAlignment;
    }

    const size_t headerSize = sizeof(AllocHeader);
    const size_t totalSize  = _size + headerSize + _align;

    void* raw = std::malloc(totalSize);
    if (!raw) {
        if (_noThrow) {
            return nullptr;
        }
        throw std::bad_alloc();
    }

    const uintptr_t rawAddr  = reinterpret_cast<uintptr_t>(raw);
    const uintptr_t userAddr = (rawAddr + headerSize + _align - 1) & ~(static_cast<uintptr_t>(_align) - 1);

    AllocHeader* header = reinterpret_cast<AllocHeader*>(userAddr) - 1;
    header->size_        = _size;
    header->offset_       = static_cast<size_t>(userAddr - rawAddr);

    g_totalAllocCount.fetch_add(1, std::memory_order_relaxed);
    g_totalAllocBytesAccum.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed);
    const int64_t current = g_currentBytes.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed) + static_cast<int64_t>(_size);
    UpdatePeak(current);

    return reinterpret_cast<void*>(userAddr);
}

/// <summary>
/// 追跡付きのメモリ解放. ヘッダからサイズと生ポインタを復元して std::free する.
/// </summary>
void TrackedFree(void* _ptr) noexcept {
    if (!_ptr) {
        return;
    }

    AllocHeader* header      = reinterpret_cast<AllocHeader*>(_ptr) - 1;
    const size_t size        = header->size_;
    const uintptr_t rawAddr  = reinterpret_cast<uintptr_t>(_ptr) - header->offset_;

    g_totalFreeCount.fetch_add(1, std::memory_order_relaxed);
    g_currentBytes.fetch_sub(static_cast<int64_t>(size), std::memory_order_relaxed);

    std::free(reinterpret_cast<void*>(rawAddr));
}

} // namespace

// --- 非aligned / non-nothrow ---
void* operator new(std::size_t _size) {
    return TrackedAllocate(_size, kMinAlignment, false);
}
void* operator new[](std::size_t _size) {
    return TrackedAllocate(_size, kMinAlignment, false);
}

// --- 非aligned / nothrow ---
void* operator new(std::size_t _size, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, kMinAlignment, true);
}
void* operator new[](std::size_t _size, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, kMinAlignment, true);
}

// --- 非aligned delete ---
void operator delete(void* _ptr) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::size_t) noexcept {
    // サイズはヘッダから復元するため引数は使用しない(sized delete対応)
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::size_t) noexcept {
    TrackedFree(_ptr);
}

// --- aligned / non-nothrow ---
void* operator new(std::size_t _size, std::align_val_t _align) {
    return TrackedAllocate(_size, static_cast<size_t>(_align), false);
}
void* operator new[](std::size_t _size, std::align_val_t _align) {
    return TrackedAllocate(_size, static_cast<size_t>(_align), false);
}

// --- aligned / nothrow ---
void* operator new(std::size_t _size, std::align_val_t _align, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, static_cast<size_t>(_align), true);
}
void* operator new[](std::size_t _size, std::align_val_t _align, const std::nothrow_t&) noexcept {
    return TrackedAllocate(_size, static_cast<size_t>(_align), true);
}

// --- aligned delete ---
void operator delete(void* _ptr, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    TrackedFree(_ptr);
}
void operator delete(void* _ptr, std::size_t, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}
void operator delete[](void* _ptr, std::size_t, std::align_val_t) noexcept {
    TrackedFree(_ptr);
}

#endif // !defined(_RELEASE)
