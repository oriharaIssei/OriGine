#include "ProfileEventStream.h"

/// stl
#include <algorithm>
#include <array>
#include <mutex>

/// engine
#include "ProfileClock.h"

/// api
// Windows.h の min/max マクロが std::min/std::max と衝突するのを防ぐ
// (Logger.h経由でDirectXヘッダがNOMINMAX未定義のまま<Windows.h>を読み込むケースがあるため、
//  このTUでは明示的に定義しておく。加えて呼び出し側も (std::min)(...) の形でマクロ展開を回避している)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace OriGine::Profiling {

namespace {
// ------------------------------------------------------------------
// スレッドストリームの登録リスト.
// 登録(スレッド初回のPROFILE_SCOPE呼び出し時のみ発生)は稀なのでmutexで保護し、
// 収集(毎フレーム, Profiler::BeginFrameから)は動的確保なしで読み取れるようにしている.
// ------------------------------------------------------------------
std::mutex g_registryMutex;
std::array<ProfileEventStream*, OriGine::Config::Profiler::kMaxThreadStreams> g_registry{};
std::atomic<size_t> g_registryCount{0};
} // namespace

ProfileEventStream::ProfileEventStream() {
    threadId_   = ::GetCurrentThreadId();
    threadName_ = "Thread-" + std::to_string(threadId_);
    Register(this);
}

void ProfileEventStream::Push(ProfileEventType _type, const char* _name) {
    const uint32_t index = writeCount_.fetch_add(1, std::memory_order_relaxed);
    if (index >= OriGine::Config::Profiler::kEventStreamCapacity) {
        // バッファ上限に達した場合は記録を諦める(オーバーフロー分は破棄する。動的確保は絶対に行わない)。
        // 破棄したことは必ず数えて外へ出す ── 黙って捨てると、Begin/Endの対応が壊れたまま
        // それらしい数字がUIに出てしまい、計測器として最悪の振る舞いになる。
        writeCount_.store(static_cast<uint32_t>(OriGine::Config::Profiler::kEventStreamCapacity), std::memory_order_relaxed);
        droppedCount_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    const uint8_t active = activeIndex_.load(std::memory_order_relaxed);
    ProfileEvent& ev      = buffers_[active][index];
    ev.timestampTicks_    = static_cast<uint64_t>(Clock::NowTicks());
    ev.threadId_          = threadId_;
    ev.type_              = _type;
    ev.SetName(_name);
}

void ProfileEventStream::PushBegin(const char* _name) {
    Push(ProfileEventType::kBegin, _name);
}

void ProfileEventStream::PushEnd(const char* _name) {
    Push(ProfileEventType::kEnd, _name);
}

void ProfileEventStream::Swap() {
    const uint8_t oldActive = activeIndex_.load(std::memory_order_relaxed);
    uint32_t written        = writeCount_.load(std::memory_order_relaxed);
    written                 = (std::min<uint32_t>)(written, static_cast<uint32_t>(OriGine::Config::Profiler::kEventStreamCapacity));

    // 書き終わったバッファを読み取り用スナップショットとして確定させる
    readIndex_   = oldActive;
    readCount_   = written;
    readDropped_ = droppedCount_.load(std::memory_order_relaxed);

    // 書き込み先を反対側のバッファへ切り替え、クリアする
    const uint8_t newActive = static_cast<uint8_t>(1 - oldActive);
    writeCount_.store(0, std::memory_order_relaxed);
    droppedCount_.store(0, std::memory_order_relaxed);
    activeIndex_.store(newActive, std::memory_order_release);
}

std::span<const ProfileEvent> ProfileEventStream::GetLastFrameEvents() const {
    return std::span<const ProfileEvent>(buffers_[readIndex_].data(), readCount_);
}

void ProfileEventStream::Register(ProfileEventStream* _stream) {
    std::lock_guard<std::mutex> lock(g_registryMutex);
    const size_t index = g_registryCount.load(std::memory_order_relaxed);
    if (index >= OriGine::Config::Profiler::kMaxThreadStreams) {
        // 想定を超えるスレッド数が登録された場合は諦める(kMaxThreadStreamsを見直すこと)
        return;
    }
    g_registry[index] = _stream;
    g_registryCount.store(index + 1, std::memory_order_release);
}

size_t ProfileEventStream::GetAllStreams(ProfileEventStream** _outArray, size_t _maxCount) {
    if (!_outArray || _maxCount == 0) {
        return 0;
    }
    const size_t count = (std::min)(g_registryCount.load(std::memory_order_acquire), _maxCount);
    for (size_t i = 0; i < count; ++i) {
        _outArray[i] = g_registry[i];
    }
    return count;
}

ProfileEventStream& ProfileEventStream::GetForCurrentThread() {
    thread_local ProfileEventStream instance;
    return instance;
}

} // namespace OriGine::Profiling
