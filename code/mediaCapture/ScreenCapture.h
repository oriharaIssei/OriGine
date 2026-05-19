#pragma once

/// microsoft
#include <wrl.h>
/// api
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_6.h>
/// stl
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace OriGine {

struct DxDevice;
class DxCommand;

struct ScreenMonitorInfo {
    uint32_t index;
    std::wstring name;
    uint32_t width;
    uint32_t height;
};

struct ScreenFrame {
    ID3D12Resource* resource = nullptr;
    const uint8_t* cpuData   = nullptr;
    uint32_t width           = 0;
    uint32_t height          = 0;
    uint32_t stride          = 0;
    uint32_t dataSize        = 0;
};

using ScreenFrameCallback = std::function<void(const ScreenFrame& frame)>;

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    static std::vector<ScreenMonitorInfo> EnumerateMonitors();

    bool Open(DxDevice* dxDevice, DxCommand* dxCommand, uint32_t monitorIndex = 0);
    void Close();

    bool StartCapture();
    void StopCapture();

    void SetFrameCallback(ScreenFrameCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    ID3D12Resource* GetCapturedResource() const { return capturedResource12_.Get(); }

    bool GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);

private:
    void CaptureThread();

    // D3D11 Desktop Duplication
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture11_;

    // Desktop Duplication
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication_;

    // Optional DX12 output resource kept for API compatibility.
    Microsoft::WRL::ComPtr<ID3D12Resource> capturedResource12_;

    uint32_t width_  = 0;
    uint32_t height_ = 0;

    std::atomic<bool> isCapturing_{false};
    std::thread captureThread_;

    ScreenFrameCallback frameCallback_;
    std::mutex callbackMutex_;

    std::mutex frameMutex_;
    std::vector<uint8_t> latestFrame_;
};

} // namespace OriGine
