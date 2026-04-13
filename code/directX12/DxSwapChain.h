#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <WinUser.h>

/// stl
#include <memory>
#include <vector>

/// engine
// directX12
#include "directX12/DxDescriptor.h"
#include "DxResource.h"
#include "EngineConfig.h"

/// math
#include "Vector4.h"

namespace OriGine {
// 前方宣言
struct DxDevice;
class DxCommand;
class WinApp;

/// <summary>
/// DirectX 12 のスワップチェーンを管理し、バックバッファの制御を行うクラス.
/// ダブルバッファリングやトリプルバッファリングをサポートし、画面への表示（Present）を担う.
/// </summary>
class DxSwapChain {
public:
    /// <summary>
    /// スワップチェーンの初期化を行う.
    /// DXGIスワップチェーンの生成と、バックバッファ用の RTV ディスクリプタの確保を含む.
    /// </summary>
    /// <param name="_winApp">ウィンドウアプリケーションのインスタンス</param>
    /// <param name="_device">使用するD3D12デバイス</param>
    /// <param name="_command">初期化コマンドの実行に使用するコマンドオブジェクト</param>
    void Initialize(const WinApp* _winApp, const DxDevice* _device, const DxCommand* _command);

    /// <summary>
    /// スワップチェーンと関連するバックバッファリソースを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// バックバッファを画面に転送（フリップ）する.
    /// </summary>
    void Present();

    /// <summary>
    /// 現在アクティブな（書き込み対象の）バックバッファを指定された色でクリアする.
    /// </summary>
    /// <param name="_commandList">実行に使用するコマンドオブジェクト</param>
    /// <param name="_dsv">クリアに使用する DSV ディスクリプタ</param>
    void CurrentBackBufferClear(DxCommand* _commandList, const DxDsvDescriptor& _dsv) const;

    /// <summary>
    /// ウィンドウサイズ変更に合わせてバックバッファをリサイズする.
    /// </summary>
    /// <param name="_width">新しい幅</param>
    /// <param name="_height">新しい高さ</param>
    void ResizeBuffer(UINT _width, UINT _height);

private:
    /// <summary>DXGIスワップチェーン本体</summary>
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    /// <summary>バックバッファ用の RTV ディスクリプタ配列</summary>
    std::vector<DxRtvDescriptor> backBuffers_;
    /// <summary>バックバッファ用のリソースオブジェクト配列</summary>
    std::vector<DxResource> backBufferResources_;
    /// <summary>構成されているバッファ数（通常 2 または 3）</summary>
    UINT bufferCount_;

    /// <summary>バッファの現在の幅</summary>
    UINT bufferWidth_ = 0;
    /// <summary>バッファの現在の高さ</summary>
    UINT bufferHeight_ = 0;

    /// <summary>画面クリア時に使用するデフォルト色</summary>
    const Vec4f clearColor_ = Config::Rendering::kDefaultClearColor;

public:
    /// <summary>構成されているバッファ数を取得する.</summary>
    UINT GetBufferCount() const { return bufferCount_; }
    /// <summary>現在書き込み対象となっているバックバッファのインデックスを取得する.</summary>
    UINT GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

    /// <summary>現在のバックバッファの RTV ディスクリプタハンドル（CPU側）を取得する.</summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtv() const {
        return backBuffers_[swapChain_->GetCurrentBackBufferIndex()].GetCpuHandle();
    }
    /// <summary>指定したインデックスのバックバッファの RTV ディスクリプタハンドル（CPU側）を取得する.</summary>
    /// <param name="_index">バッファインデックス</param>
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtv(UINT _index) const {
        if (_index >= bufferCount_) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRtv");
        }
        return backBuffers_[_index].GetCpuHandle();
    }

    /// <summary>IDXGISwapChain4 オブジェクトを取得する.</summary>
    IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }

    /// 指定したインデックスのバックバッファリソースを取得する
    /// </summary>
    /// <param name="_index">インデックス</param>
    /// <returns>リソース</returns>
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetBackBuffer(UINT _index) const { return backBufferResources_[_index].GetResource(); }
    /// <summary>
    /// 指定したインデックスのバックバッファリソースを取得する
    /// </summary>
    /// <param name="_index">インデックス</param>
    /// <returns>リソース</returns>
    Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBufferRef(UINT _index) {
        if (_index >= backBufferResources_.size()) {
            throw std::out_of_range("Index out of range in DxSwapChain::getBackBufferRef");
        }
        return backBufferResources_[_index].GetResource();
    }
    /// <summary>
    /// 現在のバックバッファリソースを取得する
    /// </summary>
    /// <returns>リソース</returns>
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetCurrentBackBuffer() const { return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].GetResource(); }
    /// <summary>
    /// 現在のバックバッファリソースを取得する
    /// </summary>
    /// <returns>リソース</returns>
    Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBufferRef() {
        return backBufferResources_[swapChain_->GetCurrentBackBufferIndex()].GetResource();
    }
};

} // namespace OriGine
