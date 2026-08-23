#include "benchmark/BenchmarkCallCounterAggregator.h"

/// stl
#include <algorithm>
#include <cstring>

using namespace OriGine;

namespace OriGine::Benchmark {

void CallCounterAggregator::AccumulateLastFrame() {
    ++countedFrames_;

    const CallCounter::FrameStat* stats = nullptr;
    const size_t statCount               = CallCounter::GetLastFrameStats(&stats);

    // CallCounter::Register()は登録済みハンドルを再利用し、新規ハンドルは末尾に追記されるだけなので
    // GetLastFrameStats()が返す配列の並びはフレームをまたいで安定している。そのため名前検索は不要で、
    // 前フレームより件数が増えていた分(=このフレームで初めて登場したカウンタ)だけ名前をコピーし、
    // 残りは値の加算のみで済む。
    for (size_t i = 0; i < statCount && i < entries_.size(); ++i) {
        if (!entries_[i].used_) {
            entries_[i].used_ = true;
            std::memcpy(entries_[i].name_, stats[i].name_, sizeof(entries_[i].name_));
        }
        entries_[i].totalCount_ += stats[i].count_;
    }
    if (statCount > entryCount_) {
        entryCount_ = statCount;
    }
}

std::vector<BenchmarkCounterStat> CallCounterAggregator::BuildResult() const {
    std::vector<BenchmarkCounterStat> result;
    result.reserve(entryCount_);

    for (size_t i = 0; i < entryCount_; ++i) {
        const Entry& e = entries_[i];
        if (!e.used_) {
            continue;
        }
        BenchmarkCounterStat stat;
        stat.name_             = e.name_;
        stat.totalCount_       = e.totalCount_;
        stat.frameSampleCount_ = countedFrames_;
        result.push_back(std::move(stat));
    }

    std::sort(result.begin(), result.end(), [](const BenchmarkCounterStat& _a, const BenchmarkCounterStat& _b) {
        return _a.totalCount_ > _b.totalCount_;
    });

    return result;
}

} // namespace OriGine::Benchmark
