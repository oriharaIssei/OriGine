#pragma once

#include <memory>
#include <wrl.h>

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "directX12/DxDescriptor.h"
#include <WinUser.h>
class DxDevice;
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
    void CurrentBackBufferClear(DxCommand* _commandList, const DxDsvDescriptor& _dsv) const;

    /// <summary>
    /// バックバッファのリサイズ
    /// </summary>
    void ResizeBuffer(UINT width, UINT height);

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    std::vector<DxRtvDescriptor> backBuffers_;
    std::vector<DxResource> backBufferResources_;
    UINT bufferCount_;

    UINT bufferWidth_  = 0;
    UINT bufferHeight_ = 0;

    const Vec4f clearColor_ = Vec4f{0.f, 0.f, 0.f, 0.0f};

public:
    UINT GetBufferCount() const { return bufferCount_; }
    UINT GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtv() const {
        return backBuffers_[swapChain_->GetCurrentBackBufferIndex()].GetCpuHandle();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv(UINT index) const {
        if (index >= bufferCount_) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRtv");
        }
        return backBuffers_[index].GetCpuHandle();
    }

    IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }

    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetBackBuffer(UINT index) const { return backBufferResources_[index].GetResource(); }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBufferRef(UINT index) {
        if (index >= backBufferResources_.size()) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRef");
        }
        return backBufferResources_[index].GetResource();
    }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetCurrentBackBuffer() const { return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].GetResource(); }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBufferRef() {
        return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].GetResource();
    }
};
