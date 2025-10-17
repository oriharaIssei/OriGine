#pragma once

#include <wrl.h>

#include "d3d12.h"

/// <summary>
/// DirectX12 Fence の WrapperClass
/// </summary>
class DxFence {
public:
    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void Finalize();
    void Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
    void WaitForFence();

private:
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceValue_;
    HANDLE fenceEvent_;

public:
    const Microsoft::WRL::ComPtr<ID3D12Fence>& getFence() const { return fence_; }
    Microsoft::WRL::ComPtr<ID3D12Fence>& getFenceRef() { return fence_; }
};
