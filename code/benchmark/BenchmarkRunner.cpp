#include "benchmark/BenchmarkRunner.h"

/// stl
#include <algorithm>
#include <chrono>

/// engine
#include "Engine.h"
#include "benchmark/BenchmarkScopeAggregator.h"
#include "scene/Scene.h"

/// profiler
#include "profiler/AllocationCounter.h"

/// externals
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Benchmark {

namespace {

/// <summary>
/// 集計対象になったフレーム(index >= warmup)の統計値からサマリを計算する
/// </summary>
void ComputeSummary(BenchmarkSummary& _summary, const std::vector<BenchmarkFrameRecord>& _frames, uint32_t _warmup) {
    std::vector<float> samples;
    samples.reserve(_frames.size());
    double allocCountSum = 0.0;
    double allocBytesSum = 0.0;

    for (const BenchmarkFrameRecord& rec : _frames) {
        if (rec.frameIndex_ < _warmup) {
            continue;
        }
        samples.push_back(static_cast<float>(rec.frameMs_));
        allocCountSum += static_cast<double>(rec.allocCount_);
        allocBytesSum += static_cast<double>(rec.allocBytes_);
    }

    _summary.sampleFrameCount_ = static_cast<uint32_t>(samples.size());
    if (samples.empty()) {
        return;
    }

    double sum = 0.0;
    for (float v : samples) {
        sum += v;
    }
    _summary.avgFrameMs_ = static_cast<float>(sum / static_cast<double>(samples.size()));

    std::vector<float> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    size_t p99Index = static_cast<size_t>(static_cast<double>(sorted.size()) * 0.99);
    if (p99Index >= sorted.size()) {
        p99Index = sorted.size() - 1;
    }
    _summary.p99FrameMs_ = sorted[p99Index];
    _summary.maxFrameMs_ = sorted.back();

    _summary.avgAllocCountPerFrame_ = allocCountSum / static_cast<double>(samples.size());
    _summary.avgAllocBytesPerFrame_ = allocBytesSum / static_cast<double>(samples.size());
}

} // namespace

BenchmarkResult RunBenchmarkLoop(Engine* _engine, Scene* _scene, const BenchmarkConfig& _config) {
    BenchmarkResult result;
    result.summary_.config_ = _config;

    if (!_engine || !_scene || _config.frames == 0) {
        LOG_ERROR("RunBenchmarkLoop: invalid arguments (engine/scene null, or frames == 0).");
        return result;
    }

    // 計測ループ内での動的確保を避けるため、必要数を事前に確保しておく
    result.frames_.reserve(_config.frames);

    ScopeAggregator scopeAggregator;

    using Clock = std::chrono::steady_clock;
    Clock::time_point prevTick{};
    bool hasPrevTick = false;
    bool aborted     = false;

    // frames+1 回だけ BeginFrame/EndFrame/Draw の完全なサイクルを回す。
    // 最後の1回はシミュレーション更新を行わない「締めのフレーム」で、
    // これによって毎回 BeginFrame と EndFrame(+Draw) が必ず対になり、
    // エンジン側のフレームライフサイクル(ImGui/コマンドリスト等)を壊さずに
    // 最終フレーム(frames-1番目)のフレームタイム/アロケーション統計まで確定させて回収できる。
    for (uint32_t frameIndex = 0; frameIndex <= _config.frames; ++frameIndex) {
        if (_engine->ProcessMessage()) {
            aborted = true;
            break;
        }

        _engine->BeginFrame();

        const Clock::time_point now = Clock::now();
        if (hasPrevTick) {
            const uint32_t completedFrameIndex = frameIndex - 1;
            const double completedFrameMs      = std::chrono::duration<double, std::milli>(now - prevTick).count();
            const AllocationCounter::FrameStats& allocStats = AllocationCounter::GetLastFrameStats();

            BenchmarkFrameRecord record;
            record.frameIndex_ = completedFrameIndex;
            record.frameMs_    = completedFrameMs;
            record.allocCount_ = allocStats.allocCount_;
            record.allocBytes_ = allocStats.allocBytes_;
            result.frames_.push_back(record);

            if (completedFrameIndex >= _config.warmup) {
                scopeAggregator.AccumulateLastFrame();
            }
        }
        prevTick    = now;
        hasPrevTick = true;

        if (frameIndex < _config.frames) {
            _scene->Update();
        }

        _engine->EndFrame();
        _engine->ScreenPreDraw();
        _engine->ScreenPostDraw();
    }

    if (aborted) {
        LOG_WARN("RunBenchmarkLoop: aborted early by window close/quit message. {} frame(s) were recorded.", result.frames_.size());
    }

    ComputeSummary(result.summary_, result.frames_, _config.warmup);
    result.scopes_ = scopeAggregator.BuildResult();

    LOG_INFO("RunBenchmarkLoop: {} frame(s) recorded ({} sampled after warmup). avg={:.4f}ms p99={:.4f}ms max={:.4f}ms",
        result.frames_.size(), result.summary_.sampleFrameCount_,
        result.summary_.avgFrameMs_, result.summary_.p99FrameMs_, result.summary_.maxFrameMs_);

    return result;
}

} // namespace OriGine::Benchmark
