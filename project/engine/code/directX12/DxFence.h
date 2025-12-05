#pragma once

/// microsoft
#include <wrl.h>
// directX12
#include "d3d12.h"

namespace OriGine {

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
    UINT64 Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);

    /// <summary>
    /// Fence の完了を待つ
    /// </summary>
    void WaitForFence(UINT64 _waitFenceVal);

private:
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceValue_;
    HANDLE fenceEvent_;

public:
};

}
