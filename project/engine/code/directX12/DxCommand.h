#pragma once

#include <unordered_map>

#include <wrl.h>

#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "directX12/DxDescriptor.h"

class DxFence;
class ResourceStateTracker;

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
    void Finalize();

public:
    static void ResetAll();
    /// <summary>
    /// Listとallocatorを同じキーで作る
    /// </summary>
    static bool CreateCommandListWithAllocator(Microsoft::WRL::ComPtr<ID3D12Device>device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType);
    static bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device>device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc);

private:
    static std::unordered_map<std::string,
        std::tuple<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, ResourceStateTracker>>
        commandListComboMap_;

    static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> commandQueueMap_;

public:
    void CommandReset();
    void ResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter);

    HRESULT Close();

    void ExecuteCommand();
    void ExecuteCommandAndPresent(IDXGISwapChain4* swapChain);

    void ClearTarget(DxRtvDescriptor* _rtv,DxDsvDescriptor* _dsv,const Vec4f& _clearColor);

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
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> getCommandList() const { return commandList_.Get(); }
    ID3D12CommandAllocator* getCommandAllocator() const { return commandAllocator_.Get(); }
    ID3D12CommandQueue* getCommandQueue() const { return commandQueue_.Get(); }
    ResourceStateTracker* getResourceStateTracker() const { return resourceStateTracker_; }
};
