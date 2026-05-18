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
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

struct WebCameraDeviceInfo {
    std::wstring id;
    std::wstring name;
};

struct CameraFrame {
    const uint8_t* data = nullptr;
    uint32_t width      = 0;
    uint32_t height     = 0;
    uint32_t stride     = 0;
    uint32_t dataSize   = 0;
};

using CameraFrameCallback = std::function<void(const CameraFrame& frame)>;

class WebCamera {
public:
    WebCamera();
    ~WebCamera();

    static void StaticInitialize();
    static void StaticFinalize();

    static std::vector<WebCameraDeviceInfo> EnumerateDevices();

    bool Open(const std::wstring& deviceId = L"", uint32_t requestWidth = 640, uint32_t requestHeight = 480);
    void Close();

    bool StartCapture();
    void StopCapture();

    void SetFrameCallback(CameraFrameCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    bool GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);

private:
    void CaptureThread();

    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader_;

    uint32_t width_  = 0;
    uint32_t height_ = 0;
    uint32_t stride_ = 0;
    bool bottomUp_   = false;
    GUID outputSubtype_ = {};

    std::atomic<bool> isCapturing_{false};
    std::thread captureThread_;

    CameraFrameCallback frameCallback_;
    std::mutex callbackMutex_;

    std::mutex frameMutex_;
    std::vector<uint8_t> latestFrame_;
};

} // namespace OriGine
