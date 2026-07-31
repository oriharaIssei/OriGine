#pragma once

/// stl
#include <array>
#include <atomic>
#include <cstdint>
#include <span>
#include <string>

/// engine
#include "EngineConfig.h"
#include "ProfileEvent.h"

namespace OriGine::Profiling {

/// <summary>
/// スレッド単位の計測イベント記録バッファ.
/// フレーム境界(Swap)でダブルバッファを入れ替えることで、
/// 「書き込み中のバッファ」と「直前フレームの完成済みスナップショット」を分離する。
/// これによりUI側は前フレームの完全なイベント列を、書き込みと競合せずに参照できる。
///
/// 記録用バッファは固定長配列(kEventStreamCapacity x 2)であり、フレーム中の
/// PushBegin/PushEnd は一切動的確保を行わない(プロファイラ自身が計測対象を汚染しないための設計)。
///
/// スレッド安全性についての注意:
/// Swap() はフレームを統括するスレッド(現状はメインスレッドのみ)から、
/// 「このストリームを書き込んでいるスレッドが当該フレームの記録を完了している」ことが
/// 保証されたタイミングでのみ呼び出すこと。現状のエンジンはシングルスレッドで
/// 1フレーム分の更新を完了させてから次の Profiler::BeginFrame() を呼ぶ構造になっているため、
/// この前提は自然に満たされている。将来ジョブシステムを導入する際は、
/// 全ジョブの完了を待ち合わせる同期点の後で Profiler::BeginFrame() を呼ぶこと。
/// </summary>
class ProfileEventStream {
public:
    ProfileEventStream();

    ProfileEventStream(const ProfileEventStream&)            = delete;
    ProfileEventStream& operator=(const ProfileEventStream&) = delete;

    /// <summary>
    /// スコープ開始イベントを記録する
    /// </summary>
    /// <param name="_name">スコープ名</param>
    void PushBegin(const char* _name);

    /// <summary>
    /// スコープ終了イベントを記録する
    /// </summary>
    /// <param name="_name">スコープ名 (Beginと対になる名前)</param>
    void PushEnd(const char* _name);

    /// <summary>
    /// フレーム境界処理. 書き込みバッファを入れ替え、新しい書き込みバッファをクリアする.
    /// 呼び出しはフレームに1回、計測を統括するスレッドから行うこと.
    /// </summary>
    void Swap();

    /// <summary>
    /// 直前フレームで記録が完了したイベント列を取得する
    /// </summary>
    /// <returns>イベント列への読み取り専用ビュー</returns>
    std::span<const ProfileEvent> GetLastFrameEvents() const;

    /// <summary>
    /// 直前フレームでバッファ上限に達して破棄されたイベント数を取得する.
    /// 0以外の場合、そのフレームのイベント列は不完全であり、
    /// 構築されるツリーの数値は信用してはならない (Begin/Endの対応が壊れているため).
    /// </summary>
    /// <returns>破棄されたイベント数</returns>
    uint32_t GetLastFrameDroppedCount() const { return readDropped_; }

    /// <summary>
    /// このストリームが紐づくスレッドID
    /// </summary>
    uint32_t GetThreadId() const { return threadId_; }

    /// <summary>
    /// 表示用のスレッド名を設定する
    /// </summary>
    /// <param name="_name">スレッド名</param>
    void SetThreadName(const std::string& _name) { threadName_ = _name; }

    /// <summary>
    /// 表示用のスレッド名を取得する
    /// </summary>
    const std::string& GetThreadName() const { return threadName_; }

    /// <summary>
    /// 現在の実行スレッドに紐づくストリームを取得する(スレッド初回呼び出し時に自動登録される)
    /// </summary>
    /// <returns>現在のスレッドに紐づくストリームへの参照</returns>
    static ProfileEventStream& GetForCurrentThread();

    /// <summary>
    /// 登録済みの全スレッドストリームを取得する(フレーム境界処理・UI表示用).
    /// 動的確保を避けるため、呼び出し側が用意した固定長バッファへ書き込む形式にしている.
    /// </summary>
    /// <param name="_outArray">格納先の配列</param>
    /// <param name="_maxCount">配列の要素数</param>
    /// <returns>実際に格納された件数</returns>
    static size_t GetAllStreams(ProfileEventStream** _outArray, size_t _maxCount);

private:
    void Push(ProfileEventType _type, const char* _name);
    static void Register(ProfileEventStream* _stream);

private:
    // ダブルバッファ本体. 動的確保はしない固定長配列.
    std::array<std::array<ProfileEvent, OriGine::Config::Profiler::kEventStreamCapacity>, 2> buffers_{};

    std::atomic<uint32_t> writeCount_{0}; // 書き込み中バッファの使用数
    std::atomic<uint32_t> droppedCount_{0}; // 書き込み中フレームで上限超過により破棄したイベント数
    std::atomic<uint8_t> activeIndex_{0}; // 現在書き込み対象のバッファIndex(0 or 1)

    uint8_t readIndex_    = 1; // 直前フレーム(読み取り用)バッファのIndex
    uint32_t readCount_   = 0; // 直前フレーム(読み取り用)バッファの使用数
    uint32_t readDropped_ = 0; // 直前フレームで破棄されたイベント数

    uint32_t threadId_ = 0;
    std::string threadName_;
};

/// <summary>
/// RAIIによる計測スコープ. コンストラクタでBeginイベント、デストラクタでEndイベントを記録する.
/// PROFILE_SCOPE マクロの実体.
/// </summary>
class ScopedEvent {
public:
    /// <summary>
    /// コンストラクタ. 現在のスレッドのストリームにBeginイベントを積む.
    /// </summary>
    /// <param name="_name">スコープ名 (このスコープが終わるまで有効なポインタであること)</param>
    explicit ScopedEvent(const char* _name)
        : stream_(&ProfileEventStream::GetForCurrentThread()), name_(_name) {
        stream_->PushBegin(name_);
    }
    ~ScopedEvent() {
        stream_->PushEnd(name_);
    }

    ScopedEvent(const ScopedEvent&)            = delete;
    ScopedEvent& operator=(const ScopedEvent&) = delete;

private:
    ProfileEventStream* stream_;
    const char* name_;
};

} // namespace OriGine::Profiling
