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

/// <summary>
/// キャプチャ対象として選択可能なモニター1台分の情報。
/// </summary>
struct ScreenMonitorInfo {
    uint32_t index;
    std::wstring name;
    uint32_t width;
    uint32_t height;
};

/// <summary>
/// 取得済みの1画面フレーム（CPU側にトップダウンBGRAとして保持）。
/// </summary>
struct ScreenFrame {
    ID3D12Resource* resource = nullptr;
    const uint8_t* cpuData   = nullptr;
    uint32_t width           = 0;
    uint32_t height          = 0;
    uint32_t stride          = 0;
    uint32_t dataSize        = 0;
};

using ScreenFrameCallback = std::function<void(const ScreenFrame& frame)>;

/// <summary>
/// デスクトップ画面をキャプチャするクラス。
/// 通常は D3D11 Desktop Duplication API を使用し、非対応環境では GDI (BitBlt) にフォールバックする。
/// 取得したフレームはCPU側バッファへコピーし、コールバック通知/ポーリングの両方で取得できる。
/// </summary>
class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    /// <summary>
    /// キャプチャ可能なモニター一覧を列挙する。
    /// </summary>
    static std::vector<ScreenMonitorInfo> EnumerateMonitors();

    /// <summary>
    /// 指定モニターをキャプチャ対象として開く。
    /// Desktop Duplication の初期化に失敗した場合は自動的に GDI キャプチャへフォールバックする。
    /// </summary>
    /// <param name="dxDevice">未使用（API互換のため保持）</param>
    /// <param name="dxCommand">未使用（API互換のため保持）</param>
    /// <param name="monitorIndex">EnumerateMonitors() が返すインデックス</param>
    /// <returns>成功したら true</returns>
    bool Open(DxDevice* dxDevice, DxCommand* dxCommand, uint32_t monitorIndex = 0);
    void Close();

    const std::string& GetLastError() const { return lastError_; }

    /// <summary>
    /// キャプチャスレッド（Duplication または GDI）を起動する。
    /// </summary>
    bool StartCapture();
    /// <summary>
    /// キャプチャスレッドを停止する。
    /// </summary>
    void StopCapture();

    void SetFrameCallback(ScreenFrameCallback callback);

    bool IsCapturing() const { return isCapturing_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    ID3D12Resource* GetCapturedResource() const { return capturedResource12_.Get(); }

    bool GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);

private:
    /// <summary>
    /// D3D11 Desktop Duplication でフレームを取得し続けるワーカースレッド本体。
    /// </summary>
    void CaptureThreadDuplication();
    /// <summary>
    /// Desktop Duplication が使えない環境向けの GDI (BitBlt) によるフォールバックキャプチャ。
    /// 約30fpsでポーリングする。
    /// </summary>
    void CaptureThreadGDI();

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
    std::string lastError_;

    // GDI fallback
    bool useGDI_ = false;
    HMONITOR targetMonitor_ = nullptr;
};

} // namespace OriGine
