#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <xaudio2.h>
/// stl
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

// 新しい映像フレーム（トップダウン BGRA / RGB32）が提示されたときに呼ばれる。
using Mp4VideoFrameCallback = std::function<void(const uint8_t* bgra, uint32_t width, uint32_t height)>;

// mp4 (H.264 + AAC) を映像＋音声同期で再生するプレイヤー。
// - 映像: IMFSourceReader で RGB32(BGRA) にデコードし、WebCamera/ScreenCapture と同じく
//   GetLatestFrame() / コールバックで供給する（アプリがテクスチャ化して描画する前提）。
// - 音声: AAC を PCM16 にデコードし、内蔵の XAudio2 ストリーミングボイスで再生する。
//   音声の再生済みサンプル数を基準クロックとして映像フレームの提示タイミングを合わせる（A/V 同期）。
//   音声ストリームが無い mp4 は壁時計を基準にする。
//
// 前提: 事前に Media Foundation を起動しておくこと（WebCamera::StaticInitialize が MFStartup を呼ぶ）。
// スレッド: デコード用と提示用の 2 スレッドを内部で回す。Open 済みなら Play/Pause/Stop/Seek で制御する。
class Mp4Player {
public:
    Mp4Player();
    ~Mp4Player();

    Mp4Player(const Mp4Player&)            = delete;
    Mp4Player& operator=(const Mp4Player&) = delete;

    /// <summary>
    /// mp4ファイルを開き、映像/音声ストリームを解析してデコード・提示用スレッドを起動する。
    /// </summary>
    /// <param name="mp4Path">再生するmp4ファイルのパス</param>
    /// <returns>成功したら true</returns>
    bool Open(const std::string& mp4Path);
    void Close();

    /// <summary>
    /// 再生を開始（またはPauseから再開）する。終端まで達していた場合は先頭から再生する。
    /// </summary>
    void Play();
    /// <summary>
    /// 再生を一時停止する。
    /// </summary>
    void Pause();
    void Stop();               // 停止して先頭へ巻き戻す
    void SetLoop(bool loop) { loop_ = loop; }
    bool Seek(double seconds); // 指定秒へシーク

    bool IsOpen() const { return reader_ != nullptr; }
    bool IsPlaying() const { return state_ == State::Playing; }
    bool HasAudio() const { return hasAudio_; }

    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }
    double GetDuration() const { return static_cast<double>(duration100ns_) / 1e7; }
    double GetPosition() const; // 秒

    // 最新の映像フレーム（トップダウン BGRA, size = width*height*4）をコピー取得する。
    bool GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);
    void SetFrameCallback(Mp4VideoFrameCallback callback);

    const std::string& GetLastError() const { return lastError_; }

private:
    enum class State { Idle, Playing, Paused, Finished };

    struct DecodedFrame {
        LONGLONG pts100ns = 0;
        std::vector<uint8_t> bgra; // トップダウン width*height*4
    };

    // 映像/音声ストリームの出力フォーマット(RGB32/PCM16)を設定し、幅・高さ・ストライド等を確定する
    bool ConfigureStreams();
    // XAudio2の再生デバイスとソースボイスを構築する
    bool CreateAudioOutput();
    // ファイルからサンプルを読み進め、シーク適用・先読みバッファ制御をしつつ音声/映像へ振り分ける
    void DecodeThread();
    // 基準クロックに従い、デコード済みキューから提示すべき映像フレームをコールバックへ供給する
    void PresentThread();
    void ApplySeekLocked(LONGLONG pos100ns); // デコードスレッドから呼ぶ
    // デコード済み音声サンプルをXAudio2ソースボイスへ送出する
    void SubmitAudio(const Microsoft::WRL::ComPtr<IMFSample>& sample);
    // デコード済み映像サンプルをBGRAへコピーしてフレームキューへ積む
    void PushVideo(const Microsoft::WRL::ComPtr<IMFSample>& sample, LONGLONG pts);
    // 音声再生位置（あれば）または壁時計から現在の基準再生位置(100ns単位)を求める
    LONGLONG GetMasterPts100ns();
    void Fail(const std::string& msg, HRESULT hr);

    Microsoft::WRL::ComPtr<IMFSourceReader> reader_;
    DWORD videoStreamIndex_ = 0;
    DWORD audioStreamIndex_ = 0;
    bool  hasVideo_ = false;
    bool  hasAudio_ = false;

    // video
    uint32_t width_    = 0;
    uint32_t height_   = 0;
    uint32_t stride_   = 0;   // abs
    bool     bottomUp_ = false;
    LONGLONG duration100ns_ = 0;

    // audio
    uint32_t audioRate_     = 0;
    uint32_t audioChannels_ = 0;

    // XAudio2 出力
    Microsoft::WRL::ComPtr<IXAudio2> xaudio_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;
    IXAudio2SourceVoice*    sourceVoice_ = nullptr;
    // 送出済みで未消費の音声バッファ（XAudio2 はコピーしないため寿命保持が必要）。デコードスレッド専用。
    std::deque<std::unique_ptr<std::vector<uint8_t>>> audioBuffers_;

    // スレッド制御
    std::thread decodeThread_;
    std::thread presentThread_;
    std::atomic<bool>  running_{false};
    std::atomic<State> state_{State::Idle};
    std::atomic<bool>  loop_{false};
    std::atomic<bool>  decodeFinished_{false};
    std::atomic<LONGLONG> pendingSeek100ns_{-1};

    // 基準クロック
    std::mutex clockMutex_;
    LONGLONG segmentStartPts_ = 0; // audio: セグメント先頭の PTS
    uint64_t baseSamples_     = 0; // audio: セグメント先頭時点の SamplesPlayed
    LARGE_INTEGER qpcFreq_{};      // no-audio 用壁時計
    LARGE_INTEGER playStartQpc_{};
    LONGLONG playedAccum100ns_ = 0;

    // 映像フレームキュー
    std::mutex frameQueueMutex_;
    std::deque<DecodedFrame> frameQueue_;

    // 最新フレーム
    std::mutex frameMutex_;
    std::vector<uint8_t> latestFrame_;

    std::mutex callbackMutex_;
    Mp4VideoFrameCallback frameCallback_;

    std::string lastError_;
};

} // namespace OriGine
