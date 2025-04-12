#include "directX12/DxCommand.h"

/// stl
// assert
#include <cassert>
/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxFence.h"

/// lib
#include "logger/Logger.h"

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> DxCommand::commandListMap_;
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> DxCommand::commandAllocatorMap_;
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> DxCommand::commandQueueMap_;

DxCommand::DxCommand() {
}

DxCommand::~DxCommand() {
}

void DxCommand::CreateCommandListWithAllocator(ID3D12Device* device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType) {
    Logger::Debug("Create CommandList : " + listAndAllocatorKey);

    commandListMap_[listAndAllocatorKey]      = nullptr;
    commandAllocatorMap_[listAndAllocatorKey] = nullptr;

    HRESULT result = device->CreateCommandAllocator(
        listType,
        IID_PPV_ARGS(&commandAllocatorMap_[listAndAllocatorKey]));
    assert(SUCCEEDED(result));

    result = device->CreateCommandList(
        0,
        listType,
        commandAllocatorMap_[listAndAllocatorKey].Get(),
        nullptr,
        IID_PPV_ARGS(&commandListMap_[listAndAllocatorKey]));
    assert(SUCCEEDED(result));
}

void DxCommand::CreateCommandQueue(ID3D12Device* device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc) {
    Logger::Debug("Create CommandQueue : " + queueKey);

    commandQueueMap_[queueKey] = nullptr;

    HRESULT result = device->CreateCommandQueue(
        &desc, IID_PPV_ARGS(&commandQueueMap_[queueKey]));
    assert(SUCCEEDED(result));
    result;
};

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey) {
    commandListKey_  = commandListKey;
    commandQueueKey_ = commandQueueKey;
    Logger::Debug("Initialize DxCommand \n CommandList  :" + commandListKey_ + "\n CommandQueue :" + commandQueueKey_);

    HRESULT result;

    ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();
    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

        result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueueMap_[commandQueueKey_]));
        assert(SUCCEEDED(result));
        ///================================================
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListMap_.count(commandListKey_) == 0) {
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================
        result = device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&commandAllocatorMap_[commandListKey_]));
        assert(SUCCEEDED(result));

        result = device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            commandAllocatorMap_[commandListKey_].Get(),
            nullptr,
            IID_PPV_ARGS(&commandListMap_[commandListKey_]));
        assert(SUCCEEDED(result));
        ///================================================
    }
    /*-----見つかった場合-----*/
    commandList_      = commandListMap_[commandListKey_];
    commandAllocator_ = commandAllocatorMap_[commandListKey_];
}

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey, D3D12_COMMAND_LIST_TYPE listType) {
    commandListKey_  = commandListKey;
    commandQueueKey_ = commandQueueKey;

    Logger::Debug("Initialize DxCommand \n CommandList  :" + commandListKey_ + "\n CommandQueue :" + commandQueueKey_);

    HRESULT result;

    ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();

    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

        result = device->CreateCommandQueue(
            &commandQueueDesc,
            IID_PPV_ARGS(&commandQueueMap_[commandQueueKey_]));
        assert(SUCCEEDED(result));
        ///================================================
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListMap_.count(commandListKey_) == 0 &&
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================
        commandAllocatorMap_.count(commandListKey_) == 0) {
        result = device->CreateCommandAllocator(
            listType,
            IID_PPV_ARGS(&commandAllocatorMap_[commandListKey_]));
        assert(SUCCEEDED(result));

        result = device->CreateCommandList(
            0,
            listType,
            commandAllocator_.Get(),
            nullptr,
            IID_PPV_ARGS(&commandListMap_[commandListKey_]));
        assert(SUCCEEDED(result));
        ///================================================
    }
    commandList_      = commandListMap_[commandListKey_];
    commandAllocator_ = commandAllocatorMap_[commandListKey_];
}

void DxCommand::CommandReset() {
    HRESULT hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));
}

void DxCommand::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) {
    commandList_->ResourceBarrier(NumBarriers, pBarriers);
}

void DxCommand::ExecuteCommand() {
    ID3D12CommandList* commandLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(1, commandLists);
}

void DxCommand::ExecuteCommandAndPresent(IDXGISwapChain4* swapChain) {
    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    ID3D12CommandList* commandLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(1, commandLists);

    HRESULT hr = swapChain->Present(1, 0);
    assert(SUCCEEDED(hr));
    hr;
    ///===============================================================
}

void DxCommand::Finalize() {
    Logger::Debug(
        "Finalize DxCommand \n CommandList Key  :" + commandListKey_ + "\n CommandQueue Key :" + commandQueueKey_);

    commandListMap_.erase(commandListKey_);
    commandAllocatorMap_.erase(commandListKey_);
    commandQueueMap_.erase(commandQueueKey_);

    commandList_.Reset();
    commandAllocator_.Reset();
    commandQueue_.Reset();

    commandListKey_  = "";
    commandQueueKey_ = "";
}

void DxCommand::ResetAll() {
    Logger::Debug("Reset All DxCommand");
    for (auto& list : commandListMap_) {
        list.second.Reset();
    }
    for (auto& allocator : commandAllocatorMap_) {
        allocator.second.Reset();
    }
    for (auto& queue : commandQueueMap_) {
        queue.second.Reset();
    }
}
