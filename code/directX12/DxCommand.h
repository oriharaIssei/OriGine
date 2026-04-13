#pragma once

#include <unordered_map>

#include <wrl.h>

#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>

/// engine
// DirectX12
#include "directX12/DxDescriptor.h"
#include "directX12/ResourceStateTracker.h"

namespace OriGine {
// 前方宣言
class DxFence;

/// <summary>
/// DirectX 12 のコマンドリスト、コマンドアロケータ、コマンドキューを組み合わせて管理するクラス.
/// キー文字列によって単一のコマンドセットを一元管理し、リソースステートの追跡機能も持つ.
/// </summary>
class DxCommand {
public:
    DxCommand();
    ~DxCommand();

    /// <summary>
    /// コマンドリストとキューを初期化する.
    /// キーが既に存在する場合はそれを使用し、存在しない場合は作成する.
    /// </summary>
    /// <param name="_commandListKey">コマンドリストとアロケータを識別する一意のキー</param>
    /// <param name="_commandQueueKey">コマンドキューを識別する一意のキー</param>
    void Initialize(const ::std::string& _commandListKey, const ::std::string& _commandQueueKey);

    /// <summary>
    /// リストタイプを指定してコマンドリストとキューを初期化する.
    /// </summary>
    /// <param name="_commandListKey">コマンドリストキー</param>
    /// <param name="_commandQueueKey">コマンドキューキー</param>
    /// <param name="_listType">D3D12_COMMAND_LIST_TYPE (DIRECT, COMPUTE, COPYなど)</param>
    void Initialize(const ::std::string& _commandListKey, const ::std::string& _commandQueueKey, D3D12_COMMAND_LIST_TYPE _listType);

    /// <summary>
    /// 本インスタンスが保持するキー情報をクリアする（実際のコマンドリソースは ResetAll で破棄する）.
    /// </summary>
    void Finalize();

public:
    /// <summary>
    /// 全ての静的マップに保持されている CommandList, Allocator, CommandQueue を破棄する.
    /// アプリケーション終了時に呼び出す必要がある.
    /// </summary>
    static void ResetAll();

    /// <summary>
    /// 指定されたキーでコマンドリストとアロケータのペアを作成し、静的マップに登録する.
    /// </summary>
    /// <param name="_device">D3D12デバイス</param>
    /// <param name="_listAndAllocatorKey">識別キー</param>
    /// <param name="_listType">コマンドリストの種類</param>
    /// <returns>成功した場合は true</returns>
    static bool CreateCommandListWithAllocator(Microsoft::WRL::ComPtr<ID3D12Device> _device, const ::std::string& _listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE _listType);

    /// <summary>
    /// 指定されたキーでコマンドキューを作成し、静的マップに登録する.
    /// </summary>
    /// <param name="_device">D3D12デバイス</param>
    /// <param name="_queueKey">識別キー</param>
    /// <param name="_desc">キューの設定情報</param>
    /// <returns>成功した場合は true</returns>
    static bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> _device, const ::std::string& _queueKey, D3D12_COMMAND_QUEUE_DESC _desc);

public:
    /// <summary>
    /// コマンドリスト、アロケータ、リソース状態トラッカーを一つにまとめた内部構造体.
    /// </summary>
    struct CommandListCombo {
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList  = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
        ResourceStateTracker resourceStateTracker;
        bool isClosed = true;
    };

private:
    /// <summary>コマンドリストコンボの一元管理マップ</summary>
    static ::std::unordered_map<::std::string, CommandListCombo> commandListComboMap_;
    /// <summary>コマンドキューの一元管理マップ</summary>
    static ::std::unordered_map<::std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> commandQueueMap_;

public:
    /// <summary>
    /// 描画コマンドの記録を開始するために、アロケータとリストをリセットする.
    /// </summary>
    void CommandReset();

    /// <summary>
    /// リソースの状態遷移（バリア）を設定する. 内部のトラッカーで現在の状態を自動判別する.
    /// </summary>
    /// <param name="_resource">対象のリソース</param>
    /// <param name="_stateAfter">遷移後の状態</param>
    void ResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_RESOURCE_STATES _stateAfter);

    /// <summary>
    /// 指定されたバリア情報をコマンドリストに直接積む.
    /// </summary>
    /// <param name="_resource">対象のリソース</param>
    /// <param name="_barrier">設定するバリア構造体</param>
    void ResourceDirectBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_RESOURCE_BARRIER _barrier);

    /// <summary>
    /// コマンドリストの記録を終了する.
    /// </summary>
    /// <returns>HRESULT</returns>
    HRESULT Close();

    /// <summary>
    /// 記録されたコマンドリストをキューに投入して実行する.
    /// </summary>
    void ExecuteCommand();

    /// <summary>
    /// コマンドを実行し、GPU側での完了を CPU が待機する（フェンス同期）.
    /// </summary>
    void ExecuteCommandAndWait();

    /// <summary>
    /// コマンドを実行し、スワップチェーンの画面転送を要求する.
    /// </summary>
    /// <param name="_swapChain">対象のスワップチェーン</param>
    void ExecuteCommandAndPresent(IDXGISwapChain4* _swapChain);

    /// <summary>
    /// レンダーターゲットと深度バッファを指定した色/値でクリアする.
    /// </summary>
    /// <param name="_rtv">クリア対象の RTV ディスクリプタ</param>
    /// <param name="_dsv">クリア対象の DSV ディスクリプタ</param>
    /// <param name="_clearColor">クリアカラー (RGBA)</param>
    void ClearTarget(const DxRtvDescriptor& _rtv, const DxDsvDescriptor& _dsv, const Vec4f& _clearColor);

private:
    /// <summary>本インスタンスが使用しているリストキー</summary>
    ::std::string commandListComboKey_;
    /// <summary>本インスタンスが使用しているキューキー</summary>
    ::std::string commandQueueKey_;

    /// <summary>操作対象のコマンドリストへの弱参照ポインタ</summary>
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList_ = nullptr;
    /// <summary>操作対象のコマンドアロケータへの弱参照ポインタ</summary>
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
    /// <summary>操作対象のコマンドキューへの弱参照ポインタ</summary>
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
    /// <summary>操作対象の状態トラッカーへの弱参照ポインタ</summary>
    ResourceStateTracker* resourceStateTracker_ = nullptr;

public:
    /// <summary>使用中のコマンドリストキーを取得する.</summary>
    const ::std::string& GetCommandListComboKey() const { return commandListComboKey_; }
    /// <summary>使用中のコマンドキューキーを取得する.</summary>
    const ::std::string& GetCommandQueueKey() const { return commandQueueKey_; }

    /// <summary>ID3D12GraphicsCommandList6 オブジェクトを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6>& GetCommandList() const { return commandList_; }
    /// <summary>ID3D12GraphicsCommandList6 オブジェクトへの参照を取得する.</summary>
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6>& GetCommandListRef() { return commandList_; }

    /// <summary>ID3D12CommandAllocator オブジェクトを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocator() const { return commandAllocator_; }
    /// <summary>ID3D12CommandAllocator オブジェクトへの参照を取得する.</summary>
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocatorRef() { return commandAllocator_; }

    /// <summary>ID3D12CommandQueue オブジェクトを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() const { return commandQueue_; }
    /// <summary>ID3D12CommandQueue オブジェクトへの参照を取得する.</summary>
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueueRef() { return commandQueue_; }

    /// <summary>リソースステートトラッカーを取得する.</summary>
    ResourceStateTracker* GetResourceStateTracker() const { return resourceStateTracker_; }
};

} // namespace OriGine
