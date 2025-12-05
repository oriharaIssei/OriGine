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

class DxFence;

namespace OriGine {

/// <summary>
/// CommandList,Allocator,CommandQueue を組み合わせて管理する
/// </summary>
class DxCommand {
public:
    DxCommand();
    ~DxCommand();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="device"></param>
    /// <param name="commandListKey">list と allocator の Key(見つからない場合,D3D12_COMMAND_LIST_TYPE_DIRECTで作成される</param>
    /// <param name="commandQueueKey"></param>
    void Initialize(const std::string& commandListKey, const std::string& commandQueueKey);
    void Initialize(const std::string& commandListKey, const std::string& commandQueueKey, D3D12_COMMAND_LIST_TYPE listType);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

public:
    /// <summary>
    /// 全てのCommandList,Allocator,CommandQueueを破棄する
    /// </summary>
    static void ResetAll();
    /// <summary>
    /// Listとallocatorを同じキーで作る
    /// </summary>
    static bool CreateCommandListWithAllocator(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType);
    /// <summary>
    /// Queueを作る
    /// </summary>
    /// <param name="device"></param>
    /// <param name="queueKey"></param>
    /// <param name="desc"></param>
    /// <returns></returns>
    static bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc);

public:
    struct CommandListCombo {
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList   = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
        ResourceStateTracker resourceStateTracker;
        bool isClosed = true;
    };

private:
    /// 一元管理用
    static std::unordered_map<std::string, CommandListCombo> commandListComboMap_;

    static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> commandQueueMap_;

public:
    void CommandReset();
    void ResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter);
    void ResourceDirectBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_BARRIER barrier);

    HRESULT Close();

    void ExecuteCommand();
    void ExecuteCommandAndPresent(IDXGISwapChain4* swapChain);

    void ClearTarget(const DxRtvDescriptor& _rtv, const DxDsvDescriptor& _dsv, const Vec4f& _clearColor);

private:
    std::string commandListComboKey_;
    std::string commandQueueKey_;

    /// <summary>
    /// commandListに対してallocatorはほぼ1対1なのでcommandListとAllocatorは同じキーで管理する
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_         = nullptr;
    ResourceStateTracker* resourceStateTracker_                      = nullptr;

public:
    const std::string& GetCommandListComboKey() const { return commandListComboKey_; }
    const std::string& GetCommandQueueKey() const { return commandQueueKey_; }

    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const { return commandList_; }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& GetCommandListRef() { return commandList_; }
    const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocator() const { return commandAllocator_; }
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocatorRef() { return commandAllocator_; }
    const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() const { return commandQueue_; }
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueueRef() { return commandQueue_; }

    ResourceStateTracker* GetResourceStateTracker() const { return resourceStateTracker_; }
};

} // namespace OriGine
