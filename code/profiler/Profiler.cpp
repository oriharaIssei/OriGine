#include "Profiler.h"

/// stl
#include <algorithm>
#include <array>

/// engine
#include "AllocationCounter.h"
#include "CallCounter.h"
#include "ProfileClock.h"

namespace OriGine {

Profiler* Profiler::GetInstance() {
    static Profiler instance;
    return &instance;
}

void Profiler::BeginFrame() {
#if !defined(_RELEASE)
    // ------------------------------------------------------------
    // フレームタイム計測 (前回BeginFrameからの経過時間)
    // ------------------------------------------------------------
    const int64_t now = Profiling::Clock::NowTicks();
    if (hasLastFrameTicks_) {
        const double ms = Profiling::Clock::TicksToMilliseconds(now - lastFrameTicks_);

        historyCursor_                    = (historyCursor_ + 1) % OriGine::Config::Profiler::kFrameHistorySize;
        frameTimeHistory_[historyCursor_] = static_cast<float>(ms);
    }
    lastFrameTicks_    = now;
    hasLastFrameTicks_ = true;

    // ------------------------------------------------------------
    // 各スレッドの計測バッファをスワップし、前フレーム分のイベント列を確定させる.
    // 動的確保を避けるため固定長配列に登録済みストリームを受け取る.
    // ------------------------------------------------------------
    std::array<Profiling::ProfileEventStream*, OriGine::Config::Profiler::kMaxThreadStreams> streams{};
    const size_t streamCount = Profiling::ProfileEventStream::GetAllStreams(streams.data(), streams.size());
    for (size_t i = 0; i < streamCount; ++i) {
        if (streams[i]) {
            streams[i]->Swap();
        }
    }

    // ------------------------------------------------------------
    // アロケーションカウンタのフレーム集計
    // ------------------------------------------------------------
    AllocationCounter::OnFrameBegin();

    // ------------------------------------------------------------
    // 呼び出し回数カウンタのフレーム集計
    // ------------------------------------------------------------
    CallCounter::OnFrameBegin();
#endif // !defined(_RELEASE)
}

void Profiler::SetCurrentThreadName(const std::string& _name) {
#if !defined(_RELEASE)
    Profiling::ProfileEventStream::GetForCurrentThread().SetThreadName(_name);
#else
    (void)_name;
#endif
}

std::vector<Profiling::ProfileThreadTree> Profiler::BuildLastFrameTrees() const {
    std::vector<Profiling::ProfileThreadTree> trees;
#if !defined(_RELEASE)
    std::array<Profiling::ProfileEventStream*, OriGine::Config::Profiler::kMaxThreadStreams> streams{};
    const size_t streamCount = Profiling::ProfileEventStream::GetAllStreams(streams.data(), streams.size());

    trees.reserve(streamCount);
    for (size_t i = 0; i < streamCount; ++i) {
        Profiling::ProfileEventStream* stream = streams[i];
        if (!stream) {
            continue;
        }
        Profiling::ProfileThreadTree tree = Profiling::BuildProfileTree(stream->GetLastFrameEvents(), stream->GetThreadName());
        tree.droppedEventCount_           = stream->GetLastFrameDroppedCount();
        trees.push_back(std::move(tree));
    }
#endif // !defined(_RELEASE)
    return trees;
}

FrameTimeStats Profiler::GetFrameTimeStats() const {
    FrameTimeStats stats;

    std::vector<float> samples;
    samples.reserve(frameTimeHistory_.size());
    for (float v : frameTimeHistory_) {
        if (v > 0.0f) {
            samples.push_back(v);
        }
    }
    if (samples.empty()) {
        return stats;
    }

    double sum = 0.0;
    float maxV = 0.0f;
    for (float v : samples) {
        sum += v;
        // Windows.hのmaxマクロとの衝突を避けるため (std::max) の形で呼び出す
        maxV = (std::max)(maxV, v);
    }
    stats.averageMs_ = static_cast<float>(sum / static_cast<double>(samples.size()));
    stats.maxMs_     = maxV;

    std::sort(samples.begin(), samples.end());
    size_t p99Index = static_cast<size_t>(static_cast<double>(samples.size()) * 0.99);
    if (p99Index >= samples.size()) {
        p99Index = samples.size() - 1;
    }
    stats.p99Ms_ = samples[p99Index];

    return stats;
}

} // namespace OriGine
