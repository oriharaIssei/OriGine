#pragma once

/// stl
#include <cstdint>
#include <type_traits>

/// engine
#include "EngineConfig.h"

namespace OriGine::Profiling {

/// <summary>
/// 計測イベントの種類
/// </summary>
enum class ProfileEventType : uint8_t {
    kBegin = 0, // スコープ開始
    kEnd   = 1, // スコープ終了
};

/// <summary>
/// 1つの計測イベント (POD / シリアライズ可能な固定長構造体).
/// 将来的にエディタを別プロセスへ分離した際、そのままバイト列として
/// ソケット等へ転送できることを意図しており、動的確保を伴うメンバ(std::string等)は持たない。
/// 「開始/終了イベントの配列」という形を崩さず、UI側から直接ポインタを触らせない設計にするための最小単位。
/// </summary>
struct ProfileEvent {
    uint64_t timestampTicks_ = 0; // QueryPerformanceCounterによる打刻値
    uint32_t threadId_       = 0; // 記録したスレッドのID (GetCurrentThreadId)
    ProfileEventType type_   = ProfileEventType::kBegin; // Begin/End
    char name_[OriGine::Config::Profiler::kEventNameCapacity]{}; // スコープ名 (固定長 / null終端 / 動的確保なし)

    /// <summary>
    /// スコープ名を安全にコピーする (バッファ長を超える場合は切り詰める。動的確保は行わない)
    /// </summary>
    /// <param name="_name">コピー元の文字列 (null終端)</param>
    void SetName(const char* _name) {
        if (!_name) {
            name_[0] = '\0';
            return;
        }
        size_t i = 0;
        for (; i + 1 < OriGine::Config::Profiler::kEventNameCapacity && _name[i] != '\0'; ++i) {
            name_[i] = _name[i];
        }
        name_[i] = '\0';
    }
};

static_assert(std::is_trivially_copyable_v<ProfileEvent>,
    "ProfileEvent はプロセス間転送を見据えたシリアライズ可能なPOD型である必要があります.");

} // namespace OriGine::Profiling
