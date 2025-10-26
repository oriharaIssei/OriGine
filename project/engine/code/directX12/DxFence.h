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

    /// <summary>
    /// コマンドキューにSignalを送る
    /// </summary>
    /// <param name="commandQueue"></param>
    void Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);

    /// <summary>
    /// Fence の完了を待つ
    /// </summary>
    void WaitForFence();

private:
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceValue_;
    HANDLE fenceEvent_;

public:
};
