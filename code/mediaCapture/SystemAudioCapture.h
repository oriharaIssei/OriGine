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
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

struct SystemAudioDeviceInfo {
    std::wstring id;
    std::wstring name;
};

struct SystemAudioFormat {
    uint32_t sampleRate    = 48000;
    uint16_t channels      = 2;
    uint16_t bitsPerSample = 16;
};

using SystemAudioDataCallback = std::function<void(const float* data, uint32_t frameCount, uint32_t channels)>;

// WASAPI ループバックで「PC がユーザーに向けて出力している音（レンダー endpoint のミックス）」を
// 取り込むキャプチャ。Microphone がマイク入力(eCapture)なのに対し、こちらは再生側(eRender)を loopback で拾う。
//
// ループバックはイベント駆動非対応のためポーリングで駆動する。さらに完全な無音時はパケットが
// 全く届かないため、経過実時間ぶんの無音サンプルを合成して供給し、音声タイムラインを実時間で
// 連続させる（録画時の映像との A/V 同期が崩れないようにするため）。
class SystemAudioCapture {
public:
    SystemAudioCapture();
    ~SystemAudioCapture();

    SystemAudioCapture(const SystemAudioCapture&)            = delete;
    SystemAudioCapture& operator=(const SystemAudioCapture&) = delete;

    // 再生(レンダー) endpoint を列挙する。
    static std::vector<SystemAudioDeviceInfo> EnumerateDevices();

    // deviceId 空でデフォルトのレンダー endpoint を loopback で開く。
    bool Open(const std::wstring& deviceId = L"");
    void Close();

    bool StartCapture();
    void StopCapture();

    void SetDataCallback(SystemAudioDataCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    const SystemAudioFormat& GetFormat() const { return format_; }

private:
    void CaptureThread();
    void ConvertCaptureData(BYTE* data, UINT32 frameCount, DWORD flags, std::vector<float>& outBuffer) const;

    Microsoft::WRL::ComPtr<IMMDevice> device_;
    Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;

    SystemAudioFormat format_;
    WAVEFORMATEX* mixFormat_ = nullptr;

    std::atomic<bool> isCapturing_{false};
    std::thread captureThread_;

    SystemAudioDataCallback dataCallback_;
    std::mutex callbackMutex_;
};

} // namespace OriGine
