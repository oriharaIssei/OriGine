#pragma once

#include <memory>
#include <wrl.h>

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "directX12/DxDescriptor.h"
#include <WinUser.h>

/// math
#include "Vector4.h"

class DxDevice;
class DxCommand;
class WinApp;
class DxSwapChain {
public:
    void Initialize(const WinApp* winApp, const DxDevice* device, const DxCommand* command);
    void Finalize();

    void Present();

    void CurrentBackBufferClear(DxCommand* _commandList, DxDsvDescriptor* _dsv) const;

    void ResizeBuffer(UINT width, UINT height);

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    std::vector<std::shared_ptr<DxRtvDescriptor>> backBuffers_;
    std::vector<DxResource> backBufferResources_;
    UINT bufferCount_;

    UINT bufferWidth_  = 0;
    UINT bufferHeight_ = 0;

    const Vec4f clearColor_ = Vec4f{0.2f, 0.2f, 0.2f, 1.0f};

public:
    UINT getBufferCount() const { return bufferCount_; }
    UINT getCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentBackBufferRtv() const {
        return backBuffers_[swapChain_->GetCurrentBackBufferIndex()]->getCpuHandle();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferRtv(UINT index) const {
        if (index >= bufferCount_) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRtv");
        }
        return backBuffers_[index]->getCpuHandle();
    }

    IDXGISwapChain4* getSwapChain() const { return swapChain_.Get(); }

    const Microsoft::WRL::ComPtr<ID3D12Resource>& getBackBuffer(UINT index) const { return backBufferResources_[index].getResource(); }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& getCurrentBackBuffer() const { return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].getResource(); }
};
