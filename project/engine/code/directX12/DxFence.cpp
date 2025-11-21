#include "directX12/DxFence.h"

#include <cassert>

void DxFence::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // 初期値0でFenceを生成
    HRESULT hr = device->CreateFence(
        fenceValue_,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));
    hr;
}

void DxFence::Finalize() {
    fence_.Reset();
}

UINT64 DxFence::Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue) {
    // コマンドキューにSignalを送る
    ++fenceValue_;
    commandQueue->Signal(fence_.Get(), fenceValue_);

    return fenceValue_;
};

void DxFence::WaitForFence(UINT64 _waitFenceVal) {
    // 完了していなければ待機
    if (fence_->GetCompletedValue() < _waitFenceVal) {
        HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        fence_->SetEventOnCompletion(_waitFenceVal, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}
