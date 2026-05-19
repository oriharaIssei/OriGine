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

struct MicrophoneDeviceInfo {
    std::wstring id;
    std::wstring name;
};

struct MicrophoneFormat {
    uint32_t sampleRate    = 44100;
    uint16_t channels      = 1;
    uint16_t bitsPerSample = 16;
};

struct MicrophoneCaptureStats {
    uint64_t packetCount = 0;
    uint64_t frameCount  = 0;
    float latestRms      = 0.0f;
    DWORD lastFlags      = 0;
    HRESULT lastError    = S_OK;
};

using MicrophoneDataCallback = std::function<void(const float* data, uint32_t frameCount, uint32_t channels)>;

class Microphone {
public:
    Microphone();
    ~Microphone();

    static std::vector<MicrophoneDeviceInfo> EnumerateDevices();

    bool Open(const std::wstring& deviceId = L"");
    void Close();

    bool StartCapture();
    void StopCapture();

    bool StartRecording(const std::string& filePath);
    void StopRecording();

    void SetDataCallback(MicrophoneDataCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    bool IsRecording() const { return isRecording_; }
    const MicrophoneFormat& GetFormat() const { return format_; }
    MicrophoneCaptureStats GetStats() const;

private:
    void CaptureThread();
    void ConvertCaptureData(BYTE* data, UINT32 frameCount, DWORD flags, std::vector<float>& outBuffer) const;
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
