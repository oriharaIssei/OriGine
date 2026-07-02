#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
/// stl
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

namespace OriGine {

// H.264 動画ストリームの設定。
struct Mp4VideoConfig {
    uint32_t width   = 0;          // 入力フレーム幅（px）
    uint32_t height  = 0;          // 入力フレーム高さ（px）
    uint32_t fps     = 30;         // 目標フレームレート
    uint32_t bitrate = 4'000'000;  // 平均ビットレート（bps）
};

// AAC 音声ストリームの設定。enabled=false で音声なし（映像のみ）の mp4 を出力する。
struct Mp4AudioConfig {
    bool     enabled    = true;
    uint32_t sampleRate = 48000;   // AAC は 44100 / 48000 のみ対応
    uint16_t channels   = 2;       // 1 or 2
    uint32_t bitrate    = 128'000; // 平均ビットレート（bps）
};

// Media Foundation SinkWriter を使い、BGRA フレームと float PCM を
// H.264 + AAC の mp4 に多重化して書き出すレコーダ。
// 映像/音声は別スレッドから push される想定で、内部で書き込みを直列化する。
// タイムスタンプは最初のサンプル到着時刻を 0 とする共通時計から自動付与する。
//
// 前提: 事前に Media Foundation を起動しておくこと（WebCamera::StaticInitialize が MFStartup を呼ぶ）。
class Mp4Recorder {
public:
    Mp4Recorder();
    ~Mp4Recorder();

    Mp4Recorder(const Mp4Recorder&)            = delete;
    Mp4Recorder& operator=(const Mp4Recorder&) = delete;

    // mp4 の書き出しを開始する。拡張子から MP4 コンテナが選択される。
    bool Open(const std::string& mp4Path, const Mp4VideoConfig& video, const Mp4AudioConfig& audio);
    // ストリームを finalize してファイルを閉じる。
    void Close();

    bool IsRecording() const { return isRecording_; }
    const std::string& GetLastError() const { return lastError_; }

    // トップダウン BGRA（RGB32）1 フレーム。byteSize は width*height*4 以上を想定。
    // 行パディングがある場合は byteSize/height を stride とみなしてコピーする。
    bool WriteVideoFrameBGRA(const uint8_t* bgra, uint32_t byteSize);

    // インターリーブされた float PCM。frameCount はチャンネルあたりのサンプル数。
    bool WriteAudioFloat(const float* interleaved, uint32_t frameCount, uint32_t channels);

private:
    bool ConfigureVideoStream();
    bool ConfigureAudioStream();
    LONGLONG NextTimestamp100ns();   // 映像用: 共通時計からの経過（100ns）
    void Fail(const std::string& msg, HRESULT hr);

    Microsoft::WRL::ComPtr<IMFSinkWriter> writer_;
    DWORD videoStream_ = 0;
    DWORD audioStream_ = 0;
    bool  hasAudio_    = false;

    Mp4VideoConfig video_;
    Mp4AudioConfig audio_;

    std::atomic<bool> isRecording_{false};

    // 共通時計。最初に到着したサンプル（映像/音声どちらでも）の時刻を基準 0 とする。
    LONGLONG startTime100ns_  = 0;
    bool     clockStarted_    = false;
    // 音声はサンプル数からギャップレスな時間軸を作る。基準時計へのオフセットを保持する。
    LONGLONG audioBaseTime_   = -1;
    uint64_t audioFramesDone_ = 0;

    std::mutex  writeMutex_;
    std::string lastError_;
};

} // namespace OriGine
