#include "SystemAudioCapture.h"

/// engine
#include "logger/Logger.h"

/// api
#include <Functiondiscoverykeys_devpkey.h>
/// stl
#include <algorithm>

#pragma comment(lib, "ole32.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

namespace {
// ループバックはポーリング駆動。無音判定・充填の閾値。
constexpr REFERENCE_TIME kBufferDuration100ns = 10'000'000; // 1s バッファ（ポーリング余裕確保）
constexpr DWORD          kPollIntervalMs      = 10;         // ~10ms 間隔でドレイン
} // namespace

SystemAudioCapture::SystemAudioCapture() {}

SystemAudioCapture::~SystemAudioCapture() {
    Close();
}

std::vector<SystemAudioDeviceInfo> SystemAudioCapture::EnumerateDevices() {
    std::vector<SystemAudioDeviceInfo> devices;

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: create enumerator failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return devices;
    }

    ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: enum render endpoints failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return devices;
    }

    UINT count = 0;
    collection->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        ComPtr<IMMDevice> device;
        if (FAILED(collection->Item(i, &device))) continue;

        LPWSTR id = nullptr;
        device->GetId(&id);

        ComPtr<IPropertyStore> props;
        device->OpenPropertyStore(STGM_READ, &props);

        PROPVARIANT varName;
        PropVariantInit(&varName);
        if (props) props->GetValue(PKEY_Device_FriendlyName, &varName);

        SystemAudioDeviceInfo info;
        info.id   = id ? id : L"";
        info.name = varName.pwszVal ? varName.pwszVal : L"Unknown";
        devices.push_back(info);

        PropVariantClear(&varName);
        if (id) CoTaskMemFree(id);
    }

    return devices;
}

bool SystemAudioCapture::Open(const std::wstring& deviceId) {
    Close();

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: create enumerator failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    // レンダー(再生)側 endpoint を取得する。これを loopback で拾う。
    if (deviceId.empty()) {
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device_);
    } else {
        hr = enumerator->GetDevice(deviceId.c_str(), &device_);
    }
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: get render endpoint failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = device_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void**>(audioClient_.GetAddressOf()));
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: activate audio client failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = audioClient_->GetMixFormat(&mixFormat_);
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: get mix format failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    format_.sampleRate    = mixFormat_->nSamplesPerSec;
    format_.channels      = static_cast<uint16_t>(mixFormat_->nChannels);
    format_.bitsPerSample = static_cast<uint16_t>(mixFormat_->wBitsPerSample);

    // ループバックはイベント駆動非対応。LOOPBACK フラグのみでポーリング取得する。
    hr = audioClient_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        kBufferDuration100ns, 0, mixFormat_, nullptr);
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: initialize (loopback) failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = audioClient_->GetService(IID_PPV_ARGS(&captureClient_));
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: get capture client failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    LOG_DEBUG("SystemAudioCapture opened (loopback): {}Hz, {}ch, {}bit",
              format_.sampleRate, format_.channels, format_.bitsPerSample);
    return true;
}

void SystemAudioCapture::Close() {
    StopCapture();

    captureClient_.Reset();
    audioClient_.Reset();
    device_.Reset();

    if (mixFormat_) {
        CoTaskMemFree(mixFormat_);
        mixFormat_ = nullptr;
    }
}

bool SystemAudioCapture::StartCapture() {
    if (!audioClient_ || isCapturing_) return false;

    isCapturing_ = true;
    HRESULT hr   = audioClient_->Start();
    if (FAILED(hr)) {
        LOG_ERROR("SystemAudioCapture: start failed: 0x{:08X}", static_cast<uint32_t>(hr));
        isCapturing_ = false;
        return false;
    }

    captureThread_ = std::thread(&SystemAudioCapture::CaptureThread, this);
    LOG_DEBUG("SystemAudioCapture capture started");
    return true;
}

void SystemAudioCapture::StopCapture() {
    if (!isCapturing_ && !captureThread_.joinable()) return;

    isCapturing_ = false;
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
    if (audioClient_) {
        audioClient_->Stop();
    }
    LOG_DEBUG("SystemAudioCapture capture stopped");
}

void SystemAudioCapture::SetDataCallback(SystemAudioDataCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    dataCallback_ = std::move(callback);
}

void SystemAudioCapture::ConvertCaptureData(BYTE* data, UINT32 frameCount, DWORD flags, std::vector<float>& outBuffer) const {
    const uint32_t sampleCount = frameCount * format_.channels;
    outBuffer.assign(sampleCount, 0.0f);

    // 無音フラグ・データ無し・サンプル無しは0埋めのまま返す
    if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 || data == nullptr || sampleCount == 0) {
        return;
    }

    // ミックスフォーマット（WAVE_FORMAT_EXTENSIBLEの場合はSubFormat）から実際のサンプル形式を判別する
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

void SystemAudioCapture::CaptureThread() {
    const uint32_t channels   = format_.channels;
    const uint32_t sampleRate = format_.sampleRate;

    LARGE_INTEGER qpcFreq{};
    QueryPerformanceFrequency(&qpcFreq);
    LARGE_INTEGER startQpc{};
    QueryPerformanceCounter(&startQpc);

    // 実時間ベースで「本来これまでに供給されているはずのフレーム数」に対し
    // 実際に供給した数を追跡し、不足分（＝無音区間）を無音で埋める。
    uint64_t framesEmitted = 0;
    std::vector<float> convertedBuffer;
    std::vector<float> silenceBuffer;

    auto emit = [&](const float* data, uint32_t frames) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (dataCallback_ && frames > 0) {
            dataCallback_(data, frames, channels);
        }
    };

    while (isCapturing_) {
        UINT32 packetLength = 0;
        HRESULT hr = captureClient_->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            Sleep(kPollIntervalMs);
            continue;
        }

        while (packetLength > 0) {
            BYTE* data       = nullptr;
            UINT32 numFrames = 0;
            DWORD flags      = 0;

            hr = captureClient_->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
            if (FAILED(hr)) break;

            ConvertCaptureData(data, numFrames, flags, convertedBuffer);
            emit(convertedBuffer.data(), numFrames);
            framesEmitted += numFrames;

            captureClient_->ReleaseBuffer(numFrames);

            hr = captureClient_->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) break;
        }

        // 無音区間の充填。経過実時間に対し供給が不足していれば無音を追加する。
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        const uint64_t elapsedFrames =
            static_cast<uint64_t>((now.QuadPart - startQpc.QuadPart) * sampleRate / qpcFreq.QuadPart);

        if (elapsedFrames > framesEmitted) {
            uint64_t gap = elapsedFrames - framesEmitted;
            // 一回の充填は最大1秒に制限（長時間ストール時の巨大確保回避。次ループで追従）。
            if (gap > sampleRate) gap = sampleRate;
            // 10ms 未満のズレはスケジューラのジッタとみなし充填しない（実音声の断片化防止）。
            if (gap >= sampleRate / 100) {
                silenceBuffer.assign(static_cast<size_t>(gap) * channels, 0.0f);
                emit(silenceBuffer.data(), static_cast<uint32_t>(gap));
                framesEmitted += gap;
            }
        }

        Sleep(kPollIntervalMs);
    }

    isCapturing_ = false;
}
