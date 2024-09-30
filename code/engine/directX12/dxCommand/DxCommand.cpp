#include "directX12/dxCommand/DxCommand.h"

#include "directX12/dxFence/DxFence.h"
#include <cassert>

std::unordered_map < std::string,Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> DxCommand::commandListMap_;
std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> DxCommand::commandAllocatorMap_;
std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D12CommandQueue>> DxCommand::commandQueueMap_;

void DxCommand::CreateCommandListWithAllocator(ID3D12Device* device,const std::string& listAndAllocatorKey,D3D12_COMMAND_LIST_TYPE listType){
	commandListMap_[listAndAllocatorKey] = nullptr;
	commandAllocatorMap_[listAndAllocatorKey] = nullptr;

	HRESULT result = device->CreateCommandAllocator(
		listType,
		IID_PPV_ARGS(&commandAllocatorMap_[listAndAllocatorKey])
	);
	assert(SUCCEEDED(result));

	result = device->CreateCommandList(
		0,
		listType,
		commandAllocatorMap_[listAndAllocatorKey].Get(),
		nullptr,
		IID_PPV_ARGS(&commandListMap_[listAndAllocatorKey])
	);
	assert(SUCCEEDED(result));
}

void DxCommand::CreateCommandQueue(ID3D12Device* device,const std::string& queueKey,D3D12_COMMAND_QUEUE_DESC desc){
	commandQueueMap_[queueKey] = nullptr;

	HRESULT result = device->CreateCommandQueue(
		&desc,IID_PPV_ARGS(&commandQueueMap_[queueKey])
	);
	assert(SUCCEEDED(result));
};

void DxCommand::Init(ID3D12Device* device,const std::string& commandListKey,const std::string& commandQueueKey){
	HRESULT result;

	if(commandQueueMap_.count(commandQueueKey) == 0){
		///================================================
		///	CommandQueue の生成
		///================================================
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

		result = device->CreateCommandQueue(&commandQueueDesc,IID_PPV_ARGS(&commandQueueMap_[commandQueueKey]));
		assert(SUCCEEDED(result));
		///================================================
	}
	commandQueue_ = commandQueueMap_[commandQueueKey];

	if(commandListMap_.count(commandListKey) == 0){
		///================================================
		///	CommandList & CommandAllocator の生成
		///================================================
		result = device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&commandAllocatorMap_[commandListKey])
		);
		assert(SUCCEEDED(result));

		result = device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			commandAllocatorMap_[commandListKey].Get(),
			nullptr,
			IID_PPV_ARGS(&commandListMap_[commandListKey])
		);
		assert(SUCCEEDED(result));
		///================================================
	}
	/*-----見つかった場合-----*/
	commandList_ = commandListMap_[commandListKey];
	commandAllocator_ = commandAllocatorMap_[commandListKey];
}

void DxCommand::Init(ID3D12Device* device,const std::string& commandListKey,const std::string& commandQueueKey,D3D12_COMMAND_LIST_TYPE listType){
	HRESULT result;

	if(commandQueueMap_.count(commandQueueKey) == 0){
		///================================================
		///	CommandQueue の生成
		///================================================
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

		result = device->CreateCommandQueue(
			&commandQueueDesc,
			IID_PPV_ARGS(&commandQueueMap_[commandQueueKey])
		);
		assert(SUCCEEDED(result));
		///================================================
	}
	commandQueue_ = commandQueueMap_[commandQueueKey];

	if(commandListMap_.count(commandListKey) == 0 &&
	   ///================================================
	   ///	CommandList & CommandAllocator の生成
	   ///================================================
	   commandAllocatorMap_.count(commandListKey) == 0){
		result = device->CreateCommandAllocator(
			listType,
			IID_PPV_ARGS(&commandAllocatorMap_[commandListKey])
		);
		assert(SUCCEEDED(result));

		result = device->CreateCommandList(
			0,
			listType,
			commandAllocator_.Get(),
			nullptr,
			IID_PPV_ARGS(&commandListMap_[commandListKey])
		);
		assert(SUCCEEDED(result));
		///================================================
	}
	commandList_ = commandListMap_[commandListKey];
	commandAllocator_ = commandAllocatorMap_[commandListKey];
}

void DxCommand::CommandReset(){
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(),nullptr);
	assert(SUCCEEDED(hr));
}

void DxCommand::ResourceBarrier(UINT NumBarriers,const D3D12_RESOURCE_BARRIER* pBarriers){
	commandList_->ResourceBarrier(NumBarriers,pBarriers);
}

void DxCommand::ExecuteCommand(){
	ID3D12CommandList* commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1,commandLists);
}

void DxCommand::ExecuteCommandAndPresent(IDXGISwapChain4* swapChain){
	///===============================================================
	/// コマンドリストの実行
	///===============================================================
	ID3D12CommandList* commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1,commandLists);

	HRESULT hr = swapChain->Present(1,0);
	assert(SUCCEEDED(hr));
	///===============================================================
}

void DxCommand::Finalize(){
	commandList_.Reset();
	commandAllocator_.Reset();
	commandQueue_.Reset();
}

void DxCommand::ResetAll(){
	for(auto& list : commandListMap_){
		list.second.Reset();
	}
	for(auto& allocator : commandAllocatorMap_){
		allocator.second.Reset();
	}
	for(auto& queue : commandQueueMap_){
		queue.second.Reset();
	}
}