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

bool DxCommand::CreateCommandListWithAllocator(ID3D12Device* device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType) {
    Logger::Debug("Create CommandList : " + listAndAllocatorKey);

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

    if (FAILED(result)) {
        Logger::Error("Failed to create command list. HRESULT: " + std::to_string(result) + "\n listAndAllocatorKey : " + listAndAllocatorKey);
        assert(false);
        return false;
    }

    return true;
}

bool DxCommand::CreateCommandQueue(ID3D12Device* device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc) {
    Logger::Debug("Create CommandQueue : " + queueKey);

    commandQueueMap_[queueKey] = nullptr;

    HRESULT result = device->CreateCommandQueue(
        &desc, IID_PPV_ARGS(&commandQueueMap_[queueKey]));

    if (FAILED(result)) {
        Logger::Error("Failed to create command queue. HRESULT: " + std::to_string(result) + "\n queueName : " + queueKey);
        assert(false);
        return false;
    }

    return true;
};

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey) {
    commandListKey_  = commandListKey;
    commandQueueKey_ = commandQueueKey;

    Logger::Debug("Initialize DxCommand \n CommandList  :" + commandListKey_ + "\n CommandQueue :" + commandQueueKey_ + "\n");

    ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();
    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);

        ///================================================
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListMap_.count(commandListKey_) == 0) {
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================

        CreateCommandListWithAllocator(device, commandListKey_, D3D12_COMMAND_LIST_TYPE_DIRECT);

        ///================================================
    }
    /*-----見つかった場合-----*/
    commandList_      = commandListMap_[commandListKey_];
    commandAllocator_ = commandAllocatorMap_[commandListKey_];
}

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey, D3D12_COMMAND_LIST_TYPE listType) {
    commandListKey_  = commandListKey;
    commandQueueKey_ = commandQueueKey;

    Logger::Debug("Initialize DxCommand \n CommandList  :" + commandListKey_ + "\n CommandQueue :" + commandQueueKey_ + "\n");

    ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();

    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListMap_.count(commandListKey_) == 0) {
        CreateCommandListWithAllocator(device, commandListKey_, listType);
    }
    commandList_      = commandListMap_[commandListKey_];
    commandAllocator_ = commandAllocatorMap_[commandListKey_];
}

void DxCommand::CommandReset() {
    HRESULT hr = commandAllocator_->Reset();
    if (FAILED(hr)) {
        Logger::Error("Failed to reset CommandAllocator. HRESULT: " + std::to_string(hr));
        assert(false);
        return;
    }

    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    if (FAILED(hr)) {
        Logger::Error("Failed to reset CommandList. HRESULT: " + std::to_string(hr));
        assert(false);
    }
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

    if (FAILED(hr)) {
        Logger::Error("Failed to present the swap chain. HRESULT: " + std::to_string(hr));

        assert(false);
    }

    hr;
    ///===============================================================
}

void DxCommand::Finalize() {
    Logger::Debug(
        "Finalize DxCommand \n CommandList Key  :" + commandListKey_ + "\n CommandQueue Key :" + commandQueueKey_ + "\n");

    // 参照カウントを取得するための関数
    auto getRefCount = [](IUnknown* ptr) -> ULONG {
        if (!ptr) {
            return 0;
        }
        ptr->AddRef(); // 一時的に参照カウントを増やす
        ULONG refCount = ptr->Release(); // Release で元に戻す
        return refCount;
    };

    ///=====================================================
    // それぞれの ComPtr の参照カウントを確認
    ///=====================================================
    ULONG listRefCount      = getRefCount(commandList_.Get());
    ULONG allocatorRefCount = getRefCount(commandAllocator_.Get());
    ULONG queueRefCount     = getRefCount(commandQueue_.Get());

    Logger::Debug(std::format("CommandList      Name {} RefCount : {}", commandListKey_, listRefCount));
    Logger::Debug(std::format("CommandAllocator Name {} RefCount : {}", commandListKey_, allocatorRefCount));
    Logger::Debug(std::format("CommandQueue     Name {} RefCount : {}", commandQueueKey_, queueRefCount));

    ///=====================================================
    // それぞれの ComPtr の参照カウントを確認して削除 (＝＝ 1 なのは 保持しているインスタンス分)
    ///=====================================================
    if (commandList_ && getRefCount(commandList_.Get()) == 1) {
        Logger::Debug("Delete CommandList : " + commandListKey_);
        commandListMap_.erase(commandListKey_);
    }

    if (commandAllocator_ && getRefCount(commandAllocator_.Get()) == 1) {
        Logger::Debug("Delete CommandAllocator : " + commandListKey_);
        commandAllocatorMap_.erase(commandListKey_);
    }

    if (commandQueue_ && getRefCount(commandQueue_.Get()) == 1) {
        Logger::Debug("Delete CommandQueue : " + commandQueueKey_);
        commandQueueMap_.erase(commandQueueKey_);
    }

    // ComPtr をリセット
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
