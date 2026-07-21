#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <Audioclient.h>
#include <mmdeviceapi.h>
/// stl
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

/// <summary>
/// マイクデバイス（WASAPIキャプチャ endpoint）1台分の識別情報。
/// </summary>
struct MicrophoneDeviceInfo {
    std::wstring id;
    std::wstring name;
};

/// <summary>
/// マイクから取得する音声フォーマット。
/// </summary>
struct MicrophoneFormat {
    uint32_t sampleRate    = 44100;
    uint16_t channels      = 1;
    uint16_t bitsPerSample = 16;
};

/// <summary>
/// キャプチャスレッドが更新する統計情報（デバッグ表示・状態監視用）。
/// </summary>
struct MicrophoneCaptureStats {
    uint64_t packetCount = 0;
    uint64_t frameCount  = 0;
    float latestRms      = 0.0f;
    DWORD lastFlags      = 0;
    HRESULT lastError    = S_OK;
};

using MicrophoneDataCallback = std::function<void(const float* data, uint32_t frameCount, uint32_t channels)>;

/// <summary>
/// WASAPI（eCapture）でマイク入力をキャプチャするクラス。
/// 内部スレッドでイベント駆動ポーリングを行い、float PCMに変換してコールバックへ供給する。
/// StartRecording/StopRecording でキャプチャ中の音声をWAVファイルへ書き出せる。
/// </summary>
class Microphone {
public:
    Microphone();
    ~Microphone();

    /// <summary>
    /// 利用可能な録音(eCapture)デバイスを列挙する。
    /// </summary>
    static std::vector<MicrophoneDeviceInfo> EnumerateDevices();

    /// <summary>
    /// 指定デバイス（空ならデフォルト）をイベント駆動の共有モードで開く。
    /// </summary>
    /// <param name="deviceId">デバイスID。空文字列でデフォルト入力を使用</param>
    /// <returns>成功したら true</returns>
    bool Open(const std::wstring& deviceId = L"");
    void Close();

    /// <summary>
    /// キャプチャスレッドを起動し、音声取得を開始する。
    /// </summary>
    bool StartCapture();
    /// <summary>
    /// キャプチャスレッドを停止する。録音中であれば録音も停止する。
    /// </summary>
    void StopCapture();

    /// <summary>
    /// キャプチャ中の音声をバッファへ蓄積し、Stop時にWAVファイルとして書き出す録音を開始する。
    /// </summary>
    /// <param name="filePath">書き出し先のWAVファイルパス</param>
    /// <returns>キャプチャが未開始の場合は false</returns>
    bool StartRecording(const std::string& filePath);
    /// <summary>
    /// 録音を終了し、蓄積したバッファをWAVファイルへ書き出す。
    /// </summary>
    void StopRecording();

    void SetDataCallback(MicrophoneDataCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    bool IsRecording() const { return isRecording_; }
    const MicrophoneFormat& GetFormat() const { return format_; }
    MicrophoneCaptureStats GetStats() const;

private:
    /// <summary>
    /// キャプチャイベントを待機し、取得したバッファを変換してコールバック通知・統計更新・
    /// （録音中なら）録音バッファへの追加を行うワーカースレッド本体。
    /// </summary>
    void CaptureThread();

    /// <summary>
    /// WASAPIから取得した生バッファ（IEEE Float / PCM 8/16/24/32bit）を
    /// -1.0〜1.0のfloat PCMへ変換する。
    /// </summary>
    void ConvertCaptureData(BYTE* data, UINT32 frameCount, DWORD flags, std::vector<float>& outBuffer) const;

    /// <summary>
    /// 16bit PCM の WAV（RIFF）ヘッダをストリームへ書き出す。
    /// </summary>
    void WriteWavHeader(std::ofstream& file, uint32_t dataSize);

    Microsoft::WRL::ComPtr<IMMDevice> device_;
    Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;
    HANDLE captureEvent_ = nullptr;

    MicrophoneFormat format_;
    WAVEFORMATEX* mixFormat_ = nullptr;

    std::atomic<bool> isCapturing_{false};
    std::atomic<bool> isRecording_{false};
    std::thread captureThread_;

    MicrophoneDataCallback dataCallback_;
    std::mutex callbackMutex_;

    mutable std::mutex statsMutex_;
    MicrophoneCaptureStats stats_;

    std::mutex recordMutex_;
    std::vector<float> recordBuffer_;
    std::string recordFilePath_;
};

} // namespace OriGine
