#include "profiler/CallCounter.h"

/// stl
#include <cstring>

/// externals
#include "logger/Logger.h"

namespace OriGine::CallCounter {

// Increment()がヘッダのinline関数から直接読み書きする実カウンタ配列の実体。
// AllocationCounterの匿名名前空間グローバルと違い、この配列だけはinline関数越しに
// 他の翻訳単位から参照される必要があるため、外部リンケージを持つ実体としてここに置く。
std::array<uint64_t, kMaxCounters> g_rawCounts{};

namespace {

// Registerの重複判定に使う「完全な名前」を保持する容量。
// EngineConfig::Profiler::kEventNameCapacity(48)はPROFILE_SCOPE用の短いスコープ名に
// 合わせた値で、"ComponentRepository::GetComponentArray<...>"のように型名を埋め込む名前だと
// プレフィックスだけで40文字近くを消費してしまう。48文字制限のまま重複判定すると、
// 例えば"Transform"と"Transform2d"が同じ切り詰め結果に丸められ、
// 型ごとに分けたいという要求そのものが壊れてしまう。
// そのため「重複判定用の完全な名前」と「表示用に切り詰めた名前(FrameStat::name_)」を
// 別バッファに分けて持つ。このコードベースで実際に出現する最長の名前
// (SkinningAnimationComponentを埋め込んだもの)でも70文字前後なので、
// 128あれば動的確保なしで衝突の心配なく識別できる。
constexpr size_t kIdentityCapacity = 128;

/// <summary>カウンタ1枠分の管理データ(重複判定用の名前のみ。実カウンタはg_rawCountsが別に持つ)</summary>
struct CounterSlot {
    char identityName_[kIdentityCapacity]{};
};

std::array<CounterSlot, kMaxCounters> g_slots{};
std::array<uint64_t, kMaxCounters> g_prevCounts{}; // 前回OnFrameBegin時点での累積値(差分計算用)
std::array<FrameStat, kMaxCounters> g_lastFrameStats{};
size_t g_registeredCount     = 0; // これまでに登録された(=重複排除後の)カウンタ数
size_t g_lastFrameStatsCount = 0; // 直近のOnFrameBegin時点でのg_registeredCountのスナップショット
bool g_overflowWarned        = false; // kMaxCounters超過の警告を1度だけ出すためのフラグ

/// <summary>
/// 固定長バッファへ動的確保なしで安全にコピーする(ProfileEvent::SetNameと同じ方式に合わせている)。
/// </summary>
void CopyTruncated(char* _dst, size_t _dstCapacity, const char* _src) {
    if (_dstCapacity == 0) {
        return;
    }
    size_t i = 0;
    for (; i + 1 < _dstCapacity && _src[i] != '\0'; ++i) {
        _dst[i] = _src[i];
    }
    _dst[i] = '\0';
}

} // namespace

Handle Register(const char* _name) {
    if (!_name) {
        _name = "";
    }

    char identity[kIdentityCapacity];
    CopyTruncated(identity, sizeof(identity), _name);

    // 同じ名前の呼び出し元は同じカウンタへ収束させる(理由はヘッダのRegister宣言のコメントを参照)。
    // 呼ばれる頻度が低く総数も256件以下に収まる前提のため、線形探索で十分。
    for (size_t i = 0; i < g_registeredCount; ++i) {
        if (std::strcmp(g_slots[i].identityName_, identity) == 0) {
            return static_cast<Handle>(i);
        }
    }

    if (g_registeredCount >= kMaxCounters) {
        if (!g_overflowWarned) {
            LOG_WARN("CallCounter: exceeded kMaxCounters ({}). '{}' and any further new counters will not be tracked.",
                kMaxCounters, _name);
            g_overflowWarned = true;
        }
        return kInvalidHandle;
    }

    const size_t index = g_registeredCount;
    std::memcpy(g_slots[index].identityName_, identity, sizeof(identity));
    CopyTruncated(g_lastFrameStats[index].name_, sizeof(g_lastFrameStats[index].name_), _name);
    g_rawCounts[index]              = 0;
    g_prevCounts[index]             = 0;
    g_lastFrameStats[index].count_ = 0;
    ++g_registeredCount;

    return static_cast<Handle>(index);
}

void OnFrameBegin() {
    for (size_t i = 0; i < g_registeredCount; ++i) {
        const uint64_t current      = g_rawCounts[i];
        g_lastFrameStats[i].count_ = current - g_prevCounts[i];
        g_prevCounts[i]              = current;
    }
    g_lastFrameStatsCount = g_registeredCount;
}

size_t GetLastFrameStats(const FrameStat** _outStats) {
    if (_outStats) {
        *_outStats = g_lastFrameStats.data();
    }
    return g_lastFrameStatsCount;
}

} // namespace OriGine::CallCounter
