#pragma once

#include <memory>
#include <wrl.h>

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "directX12/DxDescriptor.h"
#include <WinUser.h>
struct DxDevice;
class DxCommand;
class WinApp;

/// math
#include "Vector4.h"

/// <summary>
/// SwapChainの管理クラス
/// </summary>
class DxSwapChain {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="winApp"></param>
    /// <param name="device"></param>
    /// <param name="command"></param>
    void Initialize(const WinApp* winApp, const DxDevice* device, const DxCommand* command);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// バッファの入れ替え
    /// </summary>
    void Present();

    /// <summary>
    /// 現在のバックバッファをクリア
    /// </summary>
    void CurrentBackBufferClear(DxCommand* _commandList, DxDsvDescriptor* _dsv) const;

    /// <summary>
    /// バックバッファのリサイズ
    /// </summary> 
    void ResizeBuffer(UINT width, UINT height);

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    std::vector<std::shared_ptr<DxRtvDescriptor>> backBuffers_;
    std::vector<DxResource> backBufferResources_;
    UINT bufferCount_;

    UINT bufferWidth_  = 0;
    UINT bufferHeight_ = 0;

    const Vec4f clearColor_ = Vec4f{0.f, 0.f, 0.f, 1.0f};

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
    Microsoft::WRL::ComPtr<ID3D12Resource> getBackBufferRef(UINT index) {
        if (index >= backBufferResources_.size()) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRef");
        }
        return backBufferResources_[index].getResource();
    }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& getCurrentBackBuffer() const { return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].getResource(); }
    Microsoft::WRL::ComPtr<ID3D12Resource> getCurrentBackBufferRef() {
        return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].getResource();
    }
};
