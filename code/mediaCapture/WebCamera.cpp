#include "WebCamera.h"

/// engine
#include "logger/Logger.h"

/// api
#include <mferror.h>

/// stl
#include <cstdlib>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

WebCamera::WebCamera() {}

WebCamera::~WebCamera() {
    Close();
}

void WebCamera::StaticInitialize() {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        LOG_CRITICAL("Failed to initialize Media Foundation: 0x{:08X}", static_cast<uint32_t>(hr));
    }
    LOG_DEBUG("Media Foundation initialized");
}

void WebCamera::StaticFinalize() {
    MFShutdown();
    LOG_DEBUG("Media Foundation shutdown");
}

std::vector<WebCameraDeviceInfo> WebCamera::EnumerateDevices() {
    std::vector<WebCameraDeviceInfo> devices;

    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) return devices;

    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                             MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) return devices;

    IMFActivate** ppDevices = nullptr;
    UINT32 count            = 0;
    hr = MFEnumDeviceSources(attributes.Get(), &ppDevices, &count);
    if (FAILED(hr)) return devices;

    for (UINT32 i = 0; i < count; ++i) {
        WCHAR* name = nullptr;
        UINT32 nameLen = 0;
        ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &nameLen);

        WCHAR* symLink = nullptr;
        UINT32 symLinkLen = 0;
        ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symLink, &symLinkLen);

        WebCameraDeviceInfo info;
        info.name = name ? name : L"Unknown";
        info.id   = symLink ? symLink : L"";

        devices.push_back(info);

        if (name) CoTaskMemFree(name);
        if (symLink) CoTaskMemFree(symLink);
        ppDevices[i]->Release();
    }
    CoTaskMemFree(ppDevices);

    return devices;
}

bool WebCamera::Open(const std::wstring& deviceId, uint32_t requestWidth, uint32_t requestHeight) {
    Close();

    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(&attributes, 2);
    if (FAILED(hr)) return false;

    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                             MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) return false;

    ComPtr<IMFMediaSource> mediaSource;

    // 指定なしは列挙結果の先頭デバイスを使用し、指定ありはシンボリックリンクが一致するものを探す
    if (deviceId.empty()) {
        IMFActivate** ppDevices = nullptr;
        UINT32 count            = 0;
        hr = MFEnumDeviceSources(attributes.Get(), &ppDevices, &count);
        if (FAILED(hr) || count == 0) {
            LOG_ERROR("No camera devices found");
            if (ppDevices) CoTaskMemFree(ppDevices);
            return false;
        }
        hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&mediaSource));
        for (UINT32 i = 0; i < count; ++i) ppDevices[i]->Release();
        CoTaskMemFree(ppDevices);
    } else {
        hr = attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, deviceId.c_str());
        if (FAILED(hr)) return false;

        ComPtr<IMFActivate> activate;
        IMFActivate** ppDevices = nullptr;
        UINT32 count            = 0;
        hr = MFEnumDeviceSources(attributes.Get(), &ppDevices, &count);
        if (FAILED(hr) || count == 0) {
            if (ppDevices) CoTaskMemFree(ppDevices);
            return false;
        }

        for (UINT32 i = 0; i < count; ++i) {
            WCHAR* symLink = nullptr;
            UINT32 symLinkLen = 0;
            ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symLink, &symLinkLen);
            bool match = (symLink && deviceId == symLink);
            if (symLink) CoTaskMemFree(symLink);
            if (match) {
                hr = ppDevices[i]->ActivateObject(IID_PPV_ARGS(&mediaSource));
                break;
            }
        }
        for (UINT32 i = 0; i < count; ++i) ppDevices[i]->Release();
        CoTaskMemFree(ppDevices);
    }

    if (!mediaSource) {
        LOG_ERROR("Failed to create media source");
        return false;
    }

    ComPtr<IMFAttributes> readerAttributes;
    hr = MFCreateAttributes(&readerAttributes, 1);
    if (FAILED(hr)) return false;

    hr = MFCreateSourceReaderFromMediaSource(mediaSource.Get(), readerAttributes.Get(), &sourceReader_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create source reader: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    // RGB32出力を要求
    ComPtr<IMFMediaType> mediaType;
    hr = MFCreateMediaType(&mediaType);
    if (FAILED(hr)) return false;

    mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, requestWidth, requestHeight);

    hr = sourceReader_->SetCurrentMediaType(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, mediaType.Get());
    if (FAILED(hr)) {
        LOG_WARN("Requested resolution {}x{} not available, using device default", requestWidth, requestHeight);
        ComPtr<IMFMediaType> defaultType;
        MFCreateMediaType(&defaultType);
        defaultType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        defaultType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        sourceReader_->SetCurrentMediaType(
            static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, defaultType.Get());
    }

    // 実際のフォーマットを取得
    ComPtr<IMFMediaType> currentType;
    hr = sourceReader_->GetCurrentMediaType(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &currentType);
    if (SUCCEEDED(hr)) {
        MFGetAttributeSize(currentType.Get(), MF_MT_FRAME_SIZE, &width_, &height_);
        currentType->GetGUID(MF_MT_SUBTYPE, &outputSubtype_);

        UINT32 strideVal = 0;
        hr = currentType->GetUINT32(MF_MT_DEFAULT_STRIDE, &strideVal);
        if (SUCCEEDED(hr)) {
            INT32 defaultStride = static_cast<INT32>(strideVal);
            stride_ = static_cast<uint32_t>(std::abs(defaultStride));
            bottomUp_ = (defaultStride < 0);
        } else if (outputSubtype_ == MFVideoFormat_NV12) {
            stride_ = width_;
            bottomUp_ = false;
        } else {
            stride_ = width_ * 4;
            bottomUp_ = false;
        }
    }

    LOG_DEBUG("WebCamera opened: {}x{}", width_, height_);
    return true;
}

void WebCamera::Close() {
    StopCapture();
    sourceReader_.Reset();
    width_          = 0;
    height_         = 0;
    stride_         = 0;
    bottomUp_       = false;
    outputSubtype_  = {};
}

bool WebCamera::StartCapture() {
    if (!sourceReader_ || isCapturing_) return false;

    isCapturing_   = true;
    captureThread_ = std::thread(&WebCamera::CaptureThread, this);
    LOG_DEBUG("WebCamera capture started");
    return true;
}

void WebCamera::StopCapture() {
    if (!isCapturing_) return;

    isCapturing_ = false;
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
    LOG_DEBUG("WebCamera capture stopped");
}

void WebCamera::SetFrameCallback(CameraFrameCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    frameCallback_ = std::move(callback);
}

bool WebCamera::GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    if (latestFrame_.empty() || width_ == 0 || height_ == 0) return false;

    // latestFrame_ は BGRA タイトパック（width_*height_*4 バイト）である前提で
    // 消費側（CameraGatekeeper::Evaluate 等）が cv::Mat 化して SIMD で読む。
    // ストライド差異やフォーマット変更直後の過渡状態で curLength が想定サイズと
    // 食い違うと、その前提が崩れてバッファ外読み取り（アクセス違反）になるため、
    // ここで整合を検証できないフレームは破棄する（ヒープ外読み取り防止）。
    const size_t expectedSize = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4;
    if (latestFrame_.size() < expectedSize) return false;

    outBuffer = latestFrame_;
    outWidth  = width_;
    outHeight = height_;
    return true;
}

void WebCamera::CaptureThread() {
    while (isCapturing_) {
        DWORD streamIndex = 0;
        DWORD flags       = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;

        HRESULT hr = sourceReader_->ReadSample(
            static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
            0, &streamIndex, &flags, &timestamp, &sample);

        if (FAILED(hr) || !isCapturing_) break;

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;

        if (!sample) continue;

        ComPtr<IMFMediaBuffer> buffer;
        hr = sample->ConvertToContiguousBuffer(&buffer);
        if (FAILED(hr)) continue;

        BYTE* rawData   = nullptr;
        DWORD maxLength = 0;
        DWORD curLength = 0;
        hr = buffer->Lock(&rawData, &maxLength, &curLength);
        if (FAILED(hr)) continue;

        {
            std::lock_guard<std::mutex> lock(frameMutex_);
            uint32_t rowBytes = width_ * 4;

            if (outputSubtype_ == MFVideoFormat_NV12 && curLength >= stride_ * height_ * 3 / 2) {
                // NV12（Y平面 + インターリーブUV半解像度平面）を BT.601 の整数近似式で BGRA へ変換する。
                // UV平面は水平・垂直とも半分解像度のため、(x & ~1u) で偶数座標に丸めて参照する。
                latestFrame_.resize(rowBytes * height_);
                const uint8_t* yPlane  = rawData;
                const uint8_t* uvPlane = rawData + stride_ * height_;
                for (uint32_t y = 0; y < height_; ++y) {
                    for (uint32_t x = 0; x < width_; ++x) {
                        int Y = yPlane[y * stride_ + x];
                        int U = uvPlane[(y / 2) * stride_ + (x & ~1u)];
                        int V = uvPlane[(y / 2) * stride_ + (x & ~1u) + 1];
                        int C = Y - 16;
                        int D = U - 128;
                        int E = V - 128;
                        int R = (298 * C + 409 * E + 128) >> 8;
                        int G = (298 * C - 100 * D - 208 * E + 128) >> 8;
                        int B = (298 * C + 516 * D + 128) >> 8;
                        if (R < 0) R = 0; if (R > 255) R = 255;
                        if (G < 0) G = 0; if (G > 255) G = 255;
                        if (B < 0) B = 0; if (B > 255) B = 255;
                        uint32_t idx = (y * width_ + x) * 4;
                        latestFrame_[idx + 0] = static_cast<uint8_t>(B);
                        latestFrame_[idx + 1] = static_cast<uint8_t>(G);
                        latestFrame_[idx + 2] = static_cast<uint8_t>(R);
                        latestFrame_[idx + 3] = 255;
                    }
                }
            } else if (bottomUp_ && curLength >= stride_ * height_) {
                // ボトムアップ（負ストライド）フォーマットは行順を反転してトップダウンへ揃える
                latestFrame_.resize(rowBytes * height_);
                for (uint32_t y = 0; y < height_; ++y) {
                    memcpy(latestFrame_.data() + y * rowBytes,
                           rawData + (height_ - 1 - y) * stride_,
                           rowBytes);
                }
            } else {
                // 既にトップダウンRGB32等でそのまま使える形式はコピーのみ
                latestFrame_.assign(rawData, rawData + curLength);
            }
        }

        CameraFrame frame;
        frame.data     = latestFrame_.data();
        frame.width    = width_;
        frame.height   = height_;
        frame.stride   = width_ * 4;
        frame.dataSize = static_cast<uint32_t>(latestFrame_.size());

        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (frameCallback_) {
                frameCallback_(frame);
            }
        }

        buffer->Unlock();
    }
}
