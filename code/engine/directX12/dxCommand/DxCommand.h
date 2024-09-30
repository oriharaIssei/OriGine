#pragma once

#include <unordered_map>

#include <wrl.h>

#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>

class DxFence;
class DxCommand {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device"></param>
	/// <param name="commandListKey">list と allocator の Key(見つからない場合,D3D12_COMMAND_LIST_TYPE_DIRECTで作成される</param>
	/// <param name="commandQueueKey"></param>
	void Init(ID3D12Device *device,const std::string &commandListKey,const std::string &commandQueueKey);
	void Init(ID3D12Device *device,const std::string &commandListKey,const std::string &commandQueueKey,D3D12_COMMAND_LIST_TYPE listType);
	void Finalize();
public:
	static void ResetAll();
	/// <summary>
	/// Listとallocatorを同じキーで作る
	/// </summary>
	static void CreateCommandListWithAllocator(ID3D12Device *device,const std::string &listAndAllocatorKey,D3D12_COMMAND_LIST_TYPE listType);
	static void CreateCommandQueue(ID3D12Device *device,const std::string &queueKey,D3D12_COMMAND_QUEUE_DESC desc);
private:
	static std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> commandListMap_;
	static std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocatorMap_;
	static std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D12CommandQueue>> commandQueueMap_;
public:
	void CommandReset();
	void ResourceBarrier(UINT NumBarriers,const D3D12_RESOURCE_BARRIER *pBarriers);
	HRESULT Close() { return commandList_->Close(); }
	void ExecuteCommand();
	void ExecuteCommandAndPresent(IDXGISwapChain4 *swapChain);
private:
	/// <summary>
	/// commandListに対してallocatorはほぼ1対1なのでcommandListとAllocatorは同じキーで管理すること
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
public:
	ID3D12GraphicsCommandList *getCommandList()const { return commandList_.Get(); }
	ID3D12CommandAllocator *getCommandAllocator()const { return commandAllocator_.Get(); }
	ID3D12CommandQueue *getCommandQueue()const { return commandQueue_.Get(); }
};