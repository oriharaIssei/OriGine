#include "benchmark/BenchmarkScopeAggregator.h"

/// stl
#include <algorithm>
#include <cstring>
#include <span>

/// profiler
#include "profiler/ProfileClock.h"
#include "profiler/ProfileEvent.h"
#include "profiler/ProfileEventStream.h"

using namespace OriGine;

namespace OriGine::Benchmark {

ScopeAggregator::Entry* ScopeAggregator::FindOrCreate(const char* _name) {
    for (size_t i = 0; i < entryCount_; ++i) {
        if (std::strncmp(entries_[i].name_, _name, OriGine::Config::Profiler::kEventNameCapacity) == 0) {
            return &entries_[i];
        }
    }
    if (entryCount_ >= kMaxScopeCount) {
        // 想定外に多くの異なるスコープ名が出現した場合は諦める(固定長テーブルのため)。
        // ProfilerWindow等のドロップ表示と同じ「取れる範囲で計測を続ける」方針に倣う。
        return nullptr;
    }
    Entry& entry = entries_[entryCount_];
    entry.used_  = true;
    // ProfileEvent::SetNameと同様、固定長バッファへ動的確保なしで安全にコピーする
    size_t i = 0;
    for (; i + 1 < OriGine::Config::Profiler::kEventNameCapacity && _name[i] != '\0'; ++i) {
        entry.name_[i] = _name[i];
    }
    entry.name_[i] = '\0';
    ++entryCount_;
    return &entry;
}

void ScopeAggregator::AccumulateLastFrame() {
    ++countedFrames_;

    std::array<Profiling::ProfileEventStream*, OriGine::Config::Profiler::kMaxThreadStreams> streams{};
    const size_t streamCount = Profiling::ProfileEventStream::GetAllStreams(streams.data(), streams.size());

    for (size_t s = 0; s < streamCount; ++s) {
        Profiling::ProfileEventStream* stream = streams[s];
        if (!stream) {
            continue;
        }
        if (stream->GetLastFrameDroppedCount() > 0) {
            // Begin/Endの対応が壊れているフレームは信用できないため、このスレッド分は集計から除外する
            continue;
        }

        const std::span<const Profiling::ProfileEvent> events = stream->GetLastFrameEvents();

        std::array<StackFrame, kMaxStackDepth> stack{};
        size_t stackSize = 0;

        for (const Profiling::ProfileEvent& ev : events) {
            if (ev.type_ == Profiling::ProfileEventType::kBegin) {
                if (stackSize >= kMaxStackDepth) {
                    // ネストが深すぎる場合は諦める(固定長スタックのため)
                    continue;
                }
                stack[stackSize].entry_        = FindOrCreate(ev.name_);
                stack[stackSize].startTicks_   = ev.timestampTicks_;
                stack[stackSize].childAccumMs_ = 0.0;
                ++stackSize;
            } else {
                if (stackSize == 0) {
                    // 対応するBeginが無いEnd(バッファ境界で切れた等)は無視する
                    continue;
                }
                --stackSize;
                const StackFrame& top = stack[stackSize];
                const double elapsedMs = Profiling::Clock::TicksToMilliseconds(
                    static_cast<int64_t>(ev.timestampTicks_ - top.startTicks_));

                if (top.entry_) {
                    top.entry_->callCount_ += 1;
                    top.entry_->totalMsSum_ += elapsedMs;
                    top.entry_->selfMsSum_ += elapsedMs - top.childAccumMs_;
                }
                if (stackSize > 0) {
                    stack[stackSize - 1].childAccumMs_ += elapsedMs;
                }
            }
        }
    }
}

std::vector<BenchmarkScopeStat> ScopeAggregator::BuildResult() const {
    std::vector<BenchmarkScopeStat> result;
    result.reserve(entryCount_);

    for (size_t i = 0; i < entryCount_; ++i) {
        const Entry& e = entries_[i];
        if (!e.used_) {
            continue;
        }
        BenchmarkScopeStat stat;
        stat.name_             = e.name_;
        stat.callCount_        = e.callCount_;
        stat.totalMsSum_       = e.totalMsSum_;
        stat.selfMsSum_        = e.selfMsSum_;
        stat.frameSampleCount_ = countedFrames_;
        result.push_back(std::move(stat));
    }

    std::sort(result.begin(), result.end(), [](const BenchmarkScopeStat& _a, const BenchmarkScopeStat& _b) {
        return _a.totalMsSum_ > _b.totalMsSum_;
    });

    return result;
}

} // namespace OriGine::Benchmark
