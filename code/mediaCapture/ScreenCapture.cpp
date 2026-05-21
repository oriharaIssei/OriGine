#include "ScreenCapture.h"

/// engine
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "logger/Logger.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace OriGine;
using Microsoft::WRL::ComPtr;

ScreenCapture::ScreenCapture() {}

ScreenCapture::~ScreenCapture() {
    Close();
}

std::vector<ScreenMonitorInfo> ScreenCapture::EnumerateMonitors() {
    std::vector<ScreenMonitorInfo> monitors;

    ComPtr<IDXGIFactory1> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return monitors;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
        ComPtr<IDXGIOutput> output;
        for (UINT outputIndex = 0; adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND; ++outputIndex) {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);

            ScreenMonitorInfo info;
            info.index  = static_cast<uint32_t>(monitors.size());
            info.name   = desc.DeviceName;
            info.width  = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
            info.height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;

            monitors.push_back(info);
            output.Reset();
        }
        adapter.Reset();
    }

    return monitors;
}

bool ScreenCapture::Open(DxDevice* dxDevice, DxCommand* dxCommand, uint32_t monitorIndex) {
    Close();
    lastError_.clear();

    (void)dxDevice;
    (void)dxCommand;

    // EnumerateMonitors() と同じファクトリベースで検索（ハイブリッドGPU対応）
    ComPtr<IDXGIOutput> targetOutput;
    ComPtr<IDXGIAdapter1> targetAdapter;

    ComPtr<IDXGIFactory1> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        lastError_ = "Failed to create DXGI factory";
        LOG_ERROR("{}: 0x{:08X}", lastError_, static_cast<uint32_t>(hr));
        return false;
    }

    uint32_t currentIndex = 0;
    bool found = false;
    ComPtr<IDXGIAdapter1> adapter1;
    for (UINT ai = 0; factory->EnumAdapters1(ai, &adapter1) != DXGI_ERROR_NOT_FOUND && !found; ++ai) {
        ComPtr<IDXGIOutput> out;
        for (UINT oi = 0; adapter1->EnumOutputs(oi, &out) != DXGI_ERROR_NOT_FOUND; ++oi) {
            if (currentIndex == monitorIndex) {
                targetOutput = out;
                targetAdapter = adapter1;
                found = true;
                break;
            }
            ++currentIndex;
            out.Reset();
        }
        adapter1.Reset();
    }

    if (!targetOutput) {
        lastError_ = "Monitor index " + std::to_string(monitorIndex) + " not found";
        LOG_ERROR("{}", lastError_);
        return false;
    }

    DXGI_OUTPUT_DESC outputDesc;
    targetOutput->GetDesc(&outputDesc);
    width_  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    height_ = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    constexpr UINT featureLevelCount = static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0]));

    HRESULT lastDupHr = S_OK;

    // D3D11デバイスを作成して DuplicateOutput を試行するヘルパー
    auto tryCreateAndDuplicate = [&](IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType) -> bool {
        d3d11Device_.Reset();
        d3d11Context_.Reset();
        duplication_.Reset();

        D3D_FEATURE_LEVEL createdLevel{};
        UINT flags = d3d11DeviceFlags;
        hr = D3D11CreateDevice(
            adapter, driverType, nullptr, flags,
            featureLevels, featureLevelCount,
            D3D11_SDK_VERSION, &d3d11Device_, &createdLevel, &d3d11Context_);
        if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG)) {
            flags &= ~D3D11_CREATE_DEVICE_DEBUG;
            hr = D3D11CreateDevice(
                adapter, driverType, nullptr, flags,
                featureLevels, featureLevelCount,
                D3D11_SDK_VERSION, &d3d11Device_, &createdLevel, &d3d11Context_);
        }
        if (FAILED(hr)) return false;

        // DuplicateOutput1 (IDXGIOutput5) を優先
        ComPtr<IDXGIOutput5> output5;
        if (SUCCEEDED(targetOutput.As(&output5))) {
            DXGI_FORMAT fmts[] = { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
            lastDupHr = output5->DuplicateOutput1(
                d3d11Device_.Get(), 0,
                static_cast<UINT>(std::size(fmts)), fmts, &duplication_);
            if (SUCCEEDED(lastDupHr)) return true;
        }
        // フォールバック: DuplicateOutput
        ComPtr<IDXGIOutput1> output1;
        if (SUCCEEDED(targetOutput.As(&output1))) {
            lastDupHr = output1->DuplicateOutput(d3d11Device_.Get(), &duplication_);
            if (SUCCEEDED(lastDupHr)) return true;
        }
        return false;
    };

    // 1. 出力を持つアダプターで試行
    bool duplicated = tryCreateAndDuplicate(targetAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN);

    // 2. 失敗時: 全アダプターを順に試行
    if (!duplicated) {
        ComPtr<IDXGIAdapter1> fallbackAdapter;
        for (UINT ai = 0; factory->EnumAdapters1(ai, &fallbackAdapter) != DXGI_ERROR_NOT_FOUND; ++ai) {
            if (fallbackAdapter.Get() == targetAdapter.Get()) {
                fallbackAdapter.Reset();
                continue;
            }
            duplicated = tryCreateAndDuplicate(fallbackAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN);
            fallbackAdapter.Reset();
            if (duplicated) break;
        }
    }

    // 3. 失敗時: デフォルトアダプター（Windowsに選ばせる）
    if (!duplicated) {
        duplicated = tryCreateAndDuplicate(nullptr, D3D_DRIVER_TYPE_HARDWARE);
    }

    if (!duplicated) {
        // Desktop Duplication 非対応 → GDI BitBlt フォールバック
        LOG_WARN("Desktop Duplication unsupported (0x{:08X}), falling back to GDI capture",
                 static_cast<uint32_t>(lastDupHr));
        d3d11Device_.Reset();
        d3d11Context_.Reset();
        duplication_.Reset();

        // HMONITOR を取得
        targetMonitor_ = nullptr;
        POINT pt = {
            outputDesc.DesktopCoordinates.left + 1,
            outputDesc.DesktopCoordinates.top + 1
        };
        targetMonitor_ = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        if (!targetMonitor_) {
            lastError_ = "Failed to get HMONITOR for GDI fallback";
            LOG_ERROR("{}", lastError_);
            width_ = 0;
            height_ = 0;
            return false;
        }

        useGDI_ = true;
        lastError_.clear();
        LOG_DEBUG("ScreenCapture opened (GDI): monitor {} ({}x{})", monitorIndex, width_, height_);
        return true;
    }

    useGDI_ = false;

    D3D11_TEXTURE2D_DESC stagingDesc{};
    stagingDesc.Width            = width_;
    stagingDesc.Height           = height_;
    stagingDesc.MipLevels        = 1;
    stagingDesc.ArraySize        = 1;
    stagingDesc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.Usage            = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

    hr = d3d11Device_->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture11_);
    if (FAILED(hr)) {
        lastError_ = "Failed to create staging texture";
        LOG_ERROR("Failed to create staging texture: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    lastError_.clear();
    LOG_DEBUG("ScreenCapture opened: monitor {} ({}x{})", monitorIndex, width_, height_);
    return true;
}

void ScreenCapture::Close() {
    StopCapture();

    if (d3d11Context_) {
        d3d11Context_->Flush();
    }

    capturedResource12_.Reset();
    stagingTexture11_.Reset();
    duplication_.Reset();
    d3d11Context_.Reset();
    d3d11Device_.Reset();

    useGDI_ = false;
    targetMonitor_ = nullptr;

    width_  = 0;
    height_ = 0;
}

bool ScreenCapture::StartCapture() {
    if (isCapturing_) return false;
    if (!useGDI_ && !duplication_) return false;

    isCapturing_ = true;
    if (useGDI_) {
        captureThread_ = std::thread(&ScreenCapture::CaptureThreadGDI, this);
    } else {
        captureThread_ = std::thread(&ScreenCapture::CaptureThreadDuplication, this);
    }
    LOG_DEBUG("ScreenCapture started ({})", useGDI_ ? "GDI" : "Duplication");
    return true;
}

void ScreenCapture::StopCapture() {
    if (!isCapturing_) return;

    isCapturing_ = false;
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
    LOG_DEBUG("ScreenCapture stopped");
}

void ScreenCapture::SetFrameCallback(ScreenFrameCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    frameCallback_ = std::move(callback);
}

bool ScreenCapture::GetLatestFrame(std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    if (latestFrame_.empty()) return false;

    outBuffer = latestFrame_;
    outWidth  = width_;
    outHeight = height_;
    return true;
}

void ScreenCapture::CaptureThreadDuplication() {
    while (isCapturing_) {
        DXGI_OUTDUPL_FRAME_INFO frameInfo;
        ComPtr<IDXGIResource> desktopResource;

        HRESULT hr = duplication_->AcquireNextFrame(100, &frameInfo, &desktopResource);
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) continue;
        if (FAILED(hr)) {
            if (hr == DXGI_ERROR_ACCESS_LOST) {
                LOG_WARN("Desktop duplication access lost, stopping capture");
            }
            break;
        }

        if (frameInfo.LastPresentTime.QuadPart == 0) {
            duplication_->ReleaseFrame();
            continue;
        }

        // D3D11テクスチャとして取得
        ComPtr<ID3D11Texture2D> desktopTex11;
        hr = desktopResource.As(&desktopTex11);
        if (FAILED(hr)) {
            duplication_->ReleaseFrame();
            continue;
        }

        d3d11Context_->CopyResource(stagingTexture11_.Get(), desktopTex11.Get());
        duplication_->ReleaseFrame();

        D3D11_MAPPED_SUBRESOURCE mapped{};
        hr = d3d11Context_->Map(stagingTexture11_.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (SUCCEEDED(hr)) {
            uint32_t dstRowSize = width_ * 4;
            {
                std::lock_guard<std::mutex> lock(frameMutex_);
                latestFrame_.resize(dstRowSize * height_);
                const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
                uint8_t* dst       = latestFrame_.data();
                for (uint32_t row = 0; row < height_; ++row) {
                    memcpy(dst + row * dstRowSize, src + row * mapped.RowPitch, dstRowSize);
                }
            }

            d3d11Context_->Unmap(stagingTexture11_.Get(), 0);

            ScreenFrame frame;
            frame.resource = nullptr;
            frame.cpuData  = latestFrame_.data();
            frame.width    = width_;
            frame.height   = height_;
            frame.stride   = dstRowSize;
            frame.dataSize = dstRowSize * height_;

            {
                std::lock_guard<std::mutex> lock(callbackMutex_);
                if (frameCallback_) {
                    frameCallback_(frame);
                }
            }
        }
    }
}

void ScreenCapture::CaptureThreadGDI() {
    MONITORINFOEXW monInfo{};
    monInfo.cbSize = sizeof(monInfo);
    if (!GetMonitorInfoW(targetMonitor_, &monInfo)) {
        LOG_ERROR("GDI capture: GetMonitorInfo failed");
        isCapturing_ = false;
        return;
    }

    int x = monInfo.rcMonitor.left;
    int y = monInfo.rcMonitor.top;
    int w = static_cast<int>(width_);
    int h = static_cast<int>(height_);

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h; // top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hDIB = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ hOld = SelectObject(hMemDC, hDIB);

    while (isCapturing_) {
        BitBlt(hMemDC, 0, 0, w, h, hScreenDC, x, y, SRCCOPY);

        uint32_t rowSize = static_cast<uint32_t>(w) * 4;
        {
            std::lock_guard<std::mutex> lock(frameMutex_);
            latestFrame_.resize(rowSize * static_cast<uint32_t>(h));
            memcpy(latestFrame_.data(), bits, latestFrame_.size());
        }

        ScreenFrame frame;
        frame.resource = nullptr;
        frame.cpuData  = latestFrame_.data();
        frame.width    = static_cast<uint32_t>(w);
        frame.height   = static_cast<uint32_t>(h);
        frame.stride   = rowSize;
        frame.dataSize = rowSize * static_cast<uint32_t>(h);

        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (frameCallback_) {
                frameCallback_(frame);
            }
        }

        Sleep(33); // ~30fps
    }

    SelectObject(hMemDC, hOld);
    DeleteObject(hDIB);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreenDC);
}
