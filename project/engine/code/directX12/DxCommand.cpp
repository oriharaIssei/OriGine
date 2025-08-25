#include "directX12/DxCommand.h"

/// stl
// assert
#include <cassert>
/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ResourceStateTracker.h"


#include "logger/Logger.h"

std::unordered_map<std::string,
    std::tuple<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, ResourceStateTracker>>
    DxCommand::commandListComboMap_;

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> DxCommand::commandQueueMap_;

DxCommand::DxCommand() {
}

DxCommand::~DxCommand() {}

bool DxCommand::CreateCommandListWithAllocator(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType) {
    LOG_DEBUG("Create CommandList : {}", listAndAllocatorKey);

    auto& commandListCombo = commandListComboMap_[listAndAllocatorKey];

    auto& commandList      = std::get<0>(commandListCombo);
    auto& commandAllocator = std::get<1>(commandListCombo);
    HRESULT result         = device->CreateCommandAllocator(
        listType,
        IID_PPV_ARGS(&std::get<1>(commandListCombo)));
    assert(SUCCEEDED(result));

    result = device->CreateCommandList(
        0,
        listType,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList));

    if (FAILED(result)) {
        LOG_ERROR("Failed to create command list. HRESULT: {} \n listAndAllocatorKey : {}", std::to_string(result), listAndAllocatorKey);
        assert(false);
        return false;
    }

    return true;
}

bool DxCommand::CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc) {
    LOG_DEBUG("Create CommandQueue : {}", queueKey);

    commandQueueMap_[queueKey] = nullptr;

    HRESULT result = device->CreateCommandQueue(
        &desc, IID_PPV_ARGS(&commandQueueMap_[queueKey]));

    if (FAILED(result)) {
        LOG_ERROR("Failed to create command queue. HRESULT: {} \n queueName : {}", std::to_string(result), queueKey);
        assert(false);
        return false;
    }

    return true;
};

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey) {
    commandListComboKey_ = commandListKey;
    commandQueueKey_     = commandQueueKey;

    LOG_DEBUG("Initialize DxCommand \n CommandList  :{} \n CommandQueue :{} \n", commandListComboKey_, commandQueueKey_);

    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::getInstance()->getDxDevice()->getDevice();
    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);

        ///================================================
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListComboMap_.count(commandListComboKey_) == 0) {
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================

        CreateCommandListWithAllocator(device, commandListComboKey_, D3D12_COMMAND_LIST_TYPE_DIRECT);

        ///================================================
    }
    /*-----見つかった場合-----*/
    auto& commandListCombo = commandListComboMap_[commandListComboKey_];
    commandList_           = std::get<0>(commandListCombo);
    commandAllocator_      = std::get<1>(commandListCombo);
    resourceStateTracker_  = &std::get<2>(commandListCombo);
}

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey, D3D12_COMMAND_LIST_TYPE listType) {
    commandListComboKey_ = commandListKey;
    commandQueueKey_     = commandQueueKey;

    LOG_DEBUG("Initialize DxCommand \n CommandList  :{} \n CommandQueue : {}", commandListComboKey_, commandQueueKey_);

    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::getInstance()->getDxDevice()->getDevice();

    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    if (commandListComboMap_.count(commandListComboKey_) == 0) {
        CreateCommandListWithAllocator(device, commandListComboKey_, listType);
    }
    auto& commandListCombo = commandListComboMap_[commandListComboKey_];
    commandList_           = std::get<0>(commandListCombo);
    commandAllocator_      = std::get<1>(commandListCombo);
    resourceStateTracker_  = &std::get<2>(commandListCombo);
}

void DxCommand::CommandReset() {
    HRESULT hr = commandAllocator_->Reset();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to reset CommandAllocator. HRESULT: {}", std::to_string(hr));
        assert(false);
        return;
    }

    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to reset CommandList. HRESULT: {}", std::to_string(hr));
        assert(false);
    }
}

void DxCommand::ResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter) {
    if (resourceStateTracker_) {
        resourceStateTracker_->Barrier(commandList_.Get(), resource.Get(), stateAfter);
    } else {
        LOG_CRITICAL("ResourceStateTracker is not initialized.");
    }
}

void DxCommand::ResourceDirectBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_BARRIER barrier) {
    if (resourceStateTracker_) {
        resourceStateTracker_->DirectBarrier(commandList_.Get(), resource.Get(), barrier);
    } else {
        LOG_CRITICAL("ResourceStateTracker is not initialized.");
    }
}

HRESULT DxCommand::Close() {
    return commandList_->Close();
}

void DxCommand::ExecuteCommand() {
    ID3D12CommandList* commandLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(1, commandLists);
    resourceStateTracker_->CommitLocalStatesToGlobal();
}

void DxCommand::ExecuteCommandAndPresent(IDXGISwapChain4* swapChain) {
    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    ID3D12CommandList* commandLists[] = {commandList_.Get()};
    commandQueue_->ExecuteCommandLists(1, commandLists);

    HRESULT hr = swapChain->Present(1, 0);

    if (FAILED(hr)) {
        LOG_ERROR("Failed to present the swap chain. HRESULT: {}", std::to_string(hr));

        assert(false);
    }
    ///===============================================================
}

void DxCommand::ClearTarget(DxRtvDescriptor* _rtv, DxDsvDescriptor* _dsv, const Vec4f& _clearColor) {

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle           = _dsv->getCpuHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtvHandle = _rtv->getCpuHandle();

    commandList_->OMSetRenderTargets(
        1,
        &backBufferRtvHandle,
        false,
        &dsvHandle);

    commandList_->ClearRenderTargetView(
        backBufferRtvHandle, _clearColor.v, 0, nullptr);

    commandList_->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DxCommand::Finalize() {
    if (!commandList_) {
        LOG_ERROR("DxCommand::Finalize: CommandList is not initialized.");
        return;
    }
    if (!commandAllocator_) {
        LOG_ERROR("DxCommand::Finalize: CommandAllocator is not initialized.");
        return;
    }
    if (!commandQueue_) {
        LOG_ERROR("DxCommand::Finalize: CommandQueue is not initialized.");
        return;
    }


    LOG_DEBUG(
        "Finalize DxCommand \n CommandList Key : {} \n CommandQueue Key :{}",commandListComboKey_, commandQueueKey_);

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

    LOG_DEBUG("CommandList      Name {} RefCount : {}", commandListComboKey_, listRefCount);
    LOG_DEBUG("CommandAllocator Name {} RefCount : {}", commandListComboKey_, allocatorRefCount);
    LOG_DEBUG("CommandQueue     Name {} RefCount : {}", commandQueueKey_, queueRefCount);

    ///=====================================================
    // それぞれの ComPtr の参照カウントを確認して削除 (＝＝ 2 なのは this + static)
    ///=====================================================
    if (commandList_ && getRefCount(commandList_.Get()) == 2) {
        LOG_DEBUG("Delete CommandList : {}", commandListComboKey_);
        commandListComboMap_.erase(commandListComboKey_);
    }

    if (commandQueue_ && getRefCount(commandQueue_.Get()) == 2) {
        LOG_DEBUG("Delete CommandQueue : {}", commandQueueKey_);
        commandQueueMap_.erase(commandQueueKey_);
    }

    // ComPtr をリセット
    commandList_.Reset();
    commandAllocator_.Reset();
    commandQueue_.Reset();

    commandListComboKey_ = "";
    commandQueueKey_     = "";
}

void DxCommand::ResetAll() {
    LOG_DEBUG("Reset All DxCommand");
    for (auto& [key, listCombo] : commandListComboMap_) {
        auto& list      = std::get<0>(listCombo);
        auto& allocator = std::get<1>(listCombo);
        list.Reset();
        allocator.Reset();
    }
    for (auto& queue : commandQueueMap_) {
        queue.second.Reset();
    }
}
