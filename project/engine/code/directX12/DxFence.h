#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

namespace OriGine {

/// <summary>
/// DirectX 12 のフェンスオブジェクト (ID3D12Fence) をラップし、CPU と GPU の同期を管理するクラス.
/// </summary>
class DxFence {
public:
    /// <summary>
    /// フェンスオブジェクトとイベントハンドルの初期化を行う.
    /// </summary>
    /// <param name="_device">D3D12 デバイス</param>
    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device);

    /// <summary>
    /// 保持しているフェンスオブジェクトとイベントハンドルを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 指定されたコマンドキューに対して Signal を発行し、現在のフェンス値を更新する.
    /// </summary>
    /// <param name="_commandQueue">Signal を発行するキュー</param>
    /// <returns>発行された Signal の値 (WaitForFence で使用する値)</returns>
    UINT64 Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue);

    /// <summary>
    /// 指定されたフェンス値に GPU が到達するまで CPU スレッドをブロックして待機する.
    /// </summary>
    /// <param name="_waitFenceVal">待機するフェンス値</param>
    void WaitForFence(UINT64 _waitFenceVal);

private:
    /// <summary>フェンスオブジェクト本体</summary>
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    /// <summary>現在のフェンス値</summary>
    UINT64 fenceValue_;
    /// <summary>同期待ちに使用する OS イベントハンドル</summary>
    HANDLE fenceEvent_;
};

}
