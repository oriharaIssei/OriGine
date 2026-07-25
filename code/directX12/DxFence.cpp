#include "directX12/DxFence.h"

#include <cassert>

using namespace OriGine;

void DxFence::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device) {
    // 初期値0でFenceを生成
    HRESULT hr = _device->CreateFence(
        fenceValue_,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));
    hr;
}

void DxFence::Finalize() {
    fence_.Reset();
}

/// <summary>
/// コマンドキューに完了通知(Signal)を仕込み、その目印となる値を返す。
/// </summary>
/// <remarks>
/// DirectX12ではCPUがコマンドを積んだ時点では実行されておらず、GPUは非同期に処理を進める。
/// そのためCPU側は「どこまで終わったか」を知る手段が必要で、これがフェンス。
/// キューに積んだコマンドを全て処理し終えた時点で、GPUがフェンスの値をここで指定した値に
/// 書き換える。CPUは戻り値を控えておき、WaitForFence()に渡すことで完了を待てる。
/// 値を毎回インクリメントするのは、送信ごとに異なる目印を割り当てて
/// どの時点の完了を待っているのかを区別するため。
/// </remarks>
UINT64 DxFence::Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue) {
    // コマンドキューにSignalを送る
    ++fenceValue_;
    _commandQueue->Signal(fence_.Get(), fenceValue_);

    return fenceValue_;
};

/// <summary>
/// 指定した目印までGPUの処理が終わるのをCPU側で待つ。
/// </summary>
/// <remarks>
/// GPUが使用中のリソースをCPUから書き換えたり解放したりすると未定義動作になるため、
/// その前にこの関数で完了を待つ必要がある。
/// </remarks>
void DxFence::WaitForFence(UINT64 _waitFenceVal) {
    // 完了していなければ待機
    // 既に到達済みならイベントの生成も待機も無駄なので、この判定で丸ごと省略する
    if (fence_->GetCompletedValue() < _waitFenceVal) {
        // 指定値に到達した時点でシグナルされるイベントを作り、それをOSに待たせる。
        // ループでGetCompletedValue()を回し続けるとCPUを1コア占有してしまうため
        HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        fence_->SetEventOnCompletion(_waitFenceVal, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}
