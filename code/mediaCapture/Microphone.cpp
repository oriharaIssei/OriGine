#include "Microphone.h"

/// stl
#include <algorithm>
#include <cmath>
#include <fstream>
/// engine
#include "logger/Logger.h"

/// api
#include <Functiondiscoverykeys_devpkey.h>

#pragma comment(lib, "ole32.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

Microphone::Microphone() {}

Microphone::~Microphone() {
    Close();
}

std::vector<MicrophoneDeviceInfo> Microphone::EnumerateDevices() {
    std::vector<MicrophoneDeviceInfo> devices;

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create device enumerator: 0x{:08X}", static_cast<uint32_t>(hr));
        return devices;
    }

    ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &collection);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to enumerate audio endpoints: 0x{:08X}", static_cast<uint32_t>(hr));
        return devices;
    }

    UINT count = 0;
    collection->GetCount(&count);

    for (UINT i = 0; i < count; ++i) {
        ComPtr<IMMDevice> device;
        hr = collection->Item(i, &device);
        if (FAILED(hr)) continue;

        LPWSTR id = nullptr;
        device->GetId(&id);

        ComPtr<IPropertyStore> props;
        device->OpenPropertyStore(STGM_READ, &props);

        PROPVARIANT varName;
        PropVariantInit(&varName);
        props->GetValue(PKEY_Device_FriendlyName, &varName);

        MicrophoneDeviceInfo info;
        info.id   = id ? id : L"";
        info.name = varName.pwszVal ? varName.pwszVal : L"Unknown";

        devices.push_back(info);

        PropVariantClear(&varName);
        if (id) CoTaskMemFree(id);
    }

    return devices;
}

bool Microphone::Open(const std::wstring& deviceId) {
    Close();

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create device enumerator: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    if (deviceId.empty()) {
        hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device_);
    } else {
        hr = enumerator->GetDevice(deviceId.c_str(), &device_);
    }
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get audio device: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = device_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void**>(audioClient_.GetAddressOf()));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to activate audio client: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = audioClient_->GetMixFormat(&mixFormat_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get mix format: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    format_.sampleRate    = mixFormat_->nSamplesPerSec;
    format_.channels      = static_cast<uint16_t>(mixFormat_->nChannels);
    format_.bitsPerSample = static_cast<uint16_t>(mixFormat_->wBitsPerSample);

    captureEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    REFERENCE_TIME bufferDuration = 200000; // 20ms
    hr = audioClient_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration, 0, mixFormat_, nullptr);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to initialize audio client: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = audioClient_->SetEventHandle(captureEvent_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set event handle: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = audioClient_->GetService(IID_PPV_ARGS(&captureClient_));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get capture client: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    LOG_DEBUG("Microphone opened: {}Hz, {}ch, {}bit",
              format_.sampleRate, format_.channels, format_.bitsPerSample);
    return true;
}

void Microphone::Close() {
    StopCapture();

    captureClient_.Reset();
    audioClient_.Reset();
    device_.Reset();

    if (mixFormat_) {
        CoTaskMemFree(mixFormat_);
        mixFormat_ = nullptr;
    }
    if (captureEvent_) {
        CloseHandle(captureEvent_);
        captureEvent_ = nullptr;
    }
}

bool Microphone::StartCapture() {
    if (!audioClient_ || isCapturing_) return false;

    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_ = MicrophoneCaptureStats{};
    }

    isCapturing_ = true;
    HRESULT hr   = audioClient_->Start();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to start audio capture: 0x{:08X}", static_cast<uint32_t>(hr));
        isCapturing_ = false;
        return false;
    }

    captureThread_ = std::thread(&Microphone::CaptureThread, this);
    LOG_DEBUG("Microphone capture started");
    return true;
}

void Microphone::StopCapture() {
    if (!isCapturing_ && !captureThread_.joinable()) return;

    isCapturing_ = false;
    if (captureEvent_) {
        SetEvent(captureEvent_);
    }
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
    if (audioClient_) {
        audioClient_->Stop();
    }

    StopRecording();
    LOG_DEBUG("Microphone capture stopped");
}

bool Microphone::StartRecording(const std::string& filePath) {
    if (!isCapturing_) {
        LOG_ERROR("Cannot start recording: capture is not active");
        return false;
    }

    std::lock_guard<std::mutex> lock(recordMutex_);
    recordBuffer_.clear();
    recordFilePath_ = filePath;
    isRecording_    = true;
    LOG_DEBUG("Microphone recording started: {}", filePath);
    return true;
}

void Microphone::StopRecording() {
    if (!isRecording_) return;
    isRecording_ = false;

    std::lock_guard<std::mutex> lock(recordMutex_);

    if (recordBuffer_.empty() || recordFilePath_.empty()) {
        recordBuffer_.clear();
        return;
    }

    std::ofstream file(recordFilePath_, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for recording: {}", recordFilePath_);
        recordBuffer_.clear();
        return;
    }

    // float -> 16bit PCM
    std::vector<int16_t> pcmData(recordBuffer_.size());
    for (size_t i = 0; i < recordBuffer_.size(); ++i) {
        float sample = recordBuffer_[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        pcmData[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    uint32_t dataSize = static_cast<uint32_t>(pcmData.size() * sizeof(int16_t));
    WriteWavHeader(file, dataSize);
    file.write(reinterpret_cast<const char*>(pcmData.data()), dataSize);
    file.close();

    LOG_DEBUG("Recording saved: {} ({} samples)", recordFilePath_, recordBuffer_.size());
    recordBuffer_.clear();
}

void Microphone::SetDataCallback(MicrophoneDataCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    dataCallback_ = std::move(callback);
}

MicrophoneCaptureStats Microphone::GetStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void Microphone::ConvertCaptureData(BYTE* data, UINT32 frameCount, DWORD flags, std::vector<float>& outBuffer) const {
    const uint32_t sampleCount = frameCount * format_.channels;
    outBuffer.assign(sampleCount, 0.0f);

    if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 || data == nullptr || sampleCount == 0) {
        return;
    }

    const bool isExtensible = mixFormat_->wFormatTag == WAVE_FORMAT_EXTENSIBLE;
    const GUID subFormat = isExtensible
        ? reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat_)->SubFormat
        : GUID{};
    const bool isFloat = mixFormat_->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
        (isExtensible && subFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);

    if (isFloat) {
        const float* src = reinterpret_cast<const float*>(data);
        std::copy(src, src + sampleCount, outBuffer.begin());
        return;
    }

    const bool isPcm = mixFormat_->wFormatTag == WAVE_FORMAT_PCM ||
        (isExtensible && subFormat == KSDATAFORMAT_SUBTYPE_PCM);
    if (!isPcm) {
        return;
    }

    switch (mixFormat_->wBitsPerSample) {
    case 8: {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(data);
        for (uint32_t i = 0; i < sampleCount; ++i) {
            outBuffer[i] = (static_cast<float>(src[i]) - 128.0f) / 128.0f;
        }
        break;
    }
    case 16: {
        const int16_t* src = reinterpret_cast<const int16_t*>(data);
        for (uint32_t i = 0; i < sampleCount; ++i) {
            outBuffer[i] = static_cast<float>(src[i]) / 32768.0f;
        }
        break;
    }
    case 24: {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(data);
        for (uint32_t i = 0; i < sampleCount; ++i) {
            int32_t value = src[i * 3] | (src[i * 3 + 1] << 8) | (src[i * 3 + 2] << 16);
            if ((value & 0x00800000) != 0) {
                value |= 0xFF000000;
            }
            outBuffer[i] = static_cast<float>(value) / 8388608.0f;
        }
        break;
    }
    case 32: {
        const int32_t* src = reinterpret_cast<const int32_t*>(data);
        for (uint32_t i = 0; i < sampleCount; ++i) {
            outBuffer[i] = static_cast<float>(src[i]) / 2147483648.0f;
        }
        break;
    }
    default:
        break;
    }
}

void Microphone::CaptureThread() {
    while (isCapturing_) {
        DWORD waitResult = WaitForSingleObject(captureEvent_, 100);
        if (!isCapturing_) break;
        if (waitResult != WAIT_OBJECT_0) continue;

        UINT32 packetLength = 0;
        HRESULT hr = captureClient_->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            std::lock_guard<std::mutex> lock(statsMutex_);
            stats_.lastError = hr;
            continue;
        }

        while (packetLength > 0) {
            BYTE* data          = nullptr;
            UINT32 numFrames    = 0;
            DWORD flags         = 0;

            hr = captureClient_->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
            if (FAILED(hr)) {
                std::lock_guard<std::mutex> lock(statsMutex_);
                stats_.lastError = hr;
                break;
            }

            std::vector<float> convertedBuffer;
            ConvertCaptureData(data, numFrames, flags, convertedBuffer);
            const float* floatData = convertedBuffer.data();

            float rms = 0.0f;
            if (!convertedBuffer.empty()) {
                for (float sample : convertedBuffer) {
                    rms += sample * sample;
                }
                rms = std::sqrt(rms / static_cast<float>(convertedBuffer.size()));
            }

            {
                std::lock_guard<std::mutex> lock(statsMutex_);
                stats_.packetCount += 1;
                stats_.frameCount += numFrames;
                stats_.latestRms = rms;
                stats_.lastFlags = flags;
                stats_.lastError = S_OK;
            }

            {
                std::lock_guard<std::mutex> lock(callbackMutex_);
                if (dataCallback_ && !convertedBuffer.empty()) {
                    dataCallback_(floatData, numFrames, format_.channels);
                }
            }

            if (isRecording_ && !convertedBuffer.empty()) {
                std::lock_guard<std::mutex> lock(recordMutex_);
                recordBuffer_.insert(recordBuffer_.end(), floatData,
                                     floatData + numFrames * format_.channels);
            }

            captureClient_->ReleaseBuffer(numFrames);
            hr = captureClient_->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                std::lock_guard<std::mutex> lock(statsMutex_);
                stats_.lastError = hr;
                break;
            }
        }
    }

    isCapturing_ = false;
}

void Microphone::WriteWavHeader(std::ofstream& file, uint32_t dataSize) {
    uint16_t numChannels   = format_.channels;
    uint32_t sampleRate    = format_.sampleRate;
    uint16_t bitsPerSample = 16;
    uint16_t blockAlign    = numChannels * bitsPerSample / 8;
    uint32_t byteRate      = sampleRate * blockAlign;
    uint32_t chunkSize     = 36 + dataSize;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    uint32_t subchunkSize = 16;
    file.write(reinterpret_cast<const char*>(&subchunkSize), 4);
    uint16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
}
