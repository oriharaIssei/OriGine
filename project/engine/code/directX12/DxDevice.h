#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace OriGine {

/// <summary>
/// DirectX 12 の基盤となるデバイス、アダプタ、ファクトリを一括管理する構造体.
/// グラフィックスエンジンの初期化時に一つ作成され、リソース生成の起点となる.
/// </summary>
struct DxDevice {
public:
    /// <summary>
    /// DirectX 12 デバイス周りの初期化を行う.
    /// 有効なアダプタの検索、デバイスの作成、DXGIファクトリの生成を含む.
    /// </summary>
    void Initialize();

    /// <summary>
    /// デバイスとファクトリを解放する.
    /// </summary>
    void Finalize();

public:
    /// <summary>DXGIオブジェクト（スワップチェーン生成などに使用）</summary>
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
    /// <summary>使用中の物理グラフィックスアダプタ</summary>
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
    /// <summary>D3D12論理デバイス</summary>
    Microsoft::WRL::ComPtr<ID3D12Device8> device_ = nullptr;
};

} // namespace OriGine
