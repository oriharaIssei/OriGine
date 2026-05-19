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

    (void)dxCommand;

    // 対象モニターを検索
    ComPtr<IDXGIOutput> targetOutput;
    ComPtr<IDXGIAdapter1> targetAdapter;
    uint32_t currentIndex = 0;
    bool found            = false;

    ComPtr<IDXGIAdapter4> adapter = dxDevice->useAdapter_;
    ComPtr<IDXGIOutput> output;
    for (UINT oi = 0; adapter->EnumOutputs(oi, &output) != DXGI_ERROR_NOT_FOUND; ++oi) {
        if (currentIndex == monitorIndex) {
            targetOutput = output;
            adapter.As(&targetAdapter);
            found        = true;
            break;
        }
        ++currentIndex;
        output.Reset();
    }

    if (!found) {
        // アダプタ直下になければファクトリ経由で全アダプタを検索
        ComPtr<IDXGIFactory1> factory;
        CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        ComPtr<IDXGIAdapter1> adapter1;
        currentIndex = 0;
        for (UINT ai = 0; factory->EnumAdapters1(ai, &adapter1) != DXGI_ERROR_NOT_FOUND && !found; ++ai) {
            ComPtr<IDXGIOutput> out;
            for (UINT oi = 0; adapter1->EnumOutputs(oi, &out) != DXGI_ERROR_NOT_FOUND; ++oi) {
                if (currentIndex == monitorIndex) {
                    targetOutput = out;
                    targetAdapter = adapter1;
                    found        = true;
                    break;
                }
                ++currentIndex;
                out.Reset();
            }
            adapter1.Reset();
        }
    }

    if (!targetOutput) {
        LOG_ERROR("Monitor index {} not found", monitorIndex);
        return false;
    }

    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    constexpr UINT featureLevelCount = static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0]));
    D3D_FEATURE_LEVEL createdFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(
        targetAdapter.Get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        d3d11DeviceFlags,
        featureLevels,
        featureLevelCount,
        D3D11_SDK_VERSION,
        &d3d11Device_,
        &createdFeatureLevel,
        &d3d11Context_);
    if (FAILED(hr) && (d3d11DeviceFlags & D3D11_CREATE_DEVICE_DEBUG) != 0) {
        d3d11DeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDevice(
            targetAdapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            d3d11DeviceFlags,
            featureLevels,
            featureLevelCount,
            D3D11_SDK_VERSION,
            &d3d11Device_,
            &createdFeatureLevel,
            &d3d11Context_);
    }
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create D3D11 capture device: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    DXGI_OUTPUT_DESC outputDesc;
    targetOutput->GetDesc(&outputDesc);
    width_  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    height_ = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    ComPtr<IDXGIOutput1> output1;
    hr = targetOutput.As(&output1);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get IDXGIOutput1: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    hr = output1->DuplicateOutput(d3d11Device_.Get(), &duplication_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to duplicate output: 0x{:08X}. Ensure no other app is capturing.", static_cast<uint32_t>(hr));
        return false;
    }

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
        LOG_ERROR("Failed to create staging texture: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

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

    width_  = 0;
    height_ = 0;
}

bool ScreenCapture::StartCapture() {
    if (!duplication_ || isCapturing_) return false;

    isCapturing_   = true;
    captureThread_ = std::thread(&ScreenCapture::CaptureThread, this);
    LOG_DEBUG("ScreenCapture started");
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

void ScreenCapture::CaptureThread() {
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
