#include "directX12/DxCommand.h"

/// stl
// assert
#include <cassert>
/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
/// logger
#include "logger/Logger.h"

/// util
#include "DxUtil.h"
#include "StringUtil.h"

using namespace OriGine;

std::unordered_map<std::string, DxCommand::CommandListCombo> DxCommand::commandListComboMap_;
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12CommandQueue>> DxCommand::commandQueueMap_;

DxCommand::DxCommand() {}
DxCommand::~DxCommand() {}

bool DxCommand::CreateCommandListWithAllocator(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& listAndAllocatorKey, D3D12_COMMAND_LIST_TYPE listType) {
    LOG_DEBUG("Create CommandList : {}", listAndAllocatorKey);

    // キーで指定されたコマンドリストとアロケータの組み合わせを生成
    auto& commandListCombo = commandListComboMap_[listAndAllocatorKey];

    auto& commandList      = commandListCombo.commandList;
    auto& commandAllocator = commandListCombo.commandAllocator;
    HRESULT result         = device->CreateCommandAllocator(
        listType,
        IID_PPV_ARGS(commandAllocator.GetAddressOf()));
    assert(SUCCEEDED(result));

    result = device->CreateCommandList(
        0,
        listType,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.GetAddressOf()));

    if (FAILED(result)) {
        LOG_ERROR("Failed to create command list. HRESULT: {} \n listAndAllocatorKey : {}", std::to_string(result), listAndAllocatorKey);
        assert(false);
        return false;
    }

    return true;
}

bool DxCommand::CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> device, const std::string& queueKey, D3D12_COMMAND_QUEUE_DESC desc) {
    LOG_DEBUG("Create CommandQueue : {}", queueKey);

    // キーで指定されたコマンドキューを生成
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
    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::GetInstance()->GetDxDevice()->device_;

    // キーで指定したコマンドリストとコマンドキューを取得または生成
    commandListComboKey_ = commandListKey;
    commandQueueKey_     = commandQueueKey;

    LOG_DEBUG("Initialize DxCommand \n CommandList  :{} \n CommandQueue :{} \n", commandListComboKey_, commandQueueKey_);

    /*-----見つからなかった場合-----*/
    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);

        ///================================================
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    /*-----見つからなかった場合-----*/
    if (commandListComboMap_.count(commandListComboKey_) == 0) {
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================

        CreateCommandListWithAllocator(device, commandListComboKey_, D3D12_COMMAND_LIST_TYPE_DIRECT);

        ///================================================
    }

    // 取得
    auto& commandListCombo = commandListComboMap_[commandListComboKey_];
    commandList_           = commandListCombo.commandList;
    commandAllocator_      = commandListCombo.commandAllocator;
    resourceStateTracker_  = &commandListCombo.resourceStateTracker;

    // 名前を設定
    commandList_->SetName(ConvertString(commandListComboKey_).c_str());
    commandAllocator_->SetName(ConvertString(commandListComboKey_).c_str());
    commandQueue_->SetName(ConvertString(commandQueueKey_).c_str());
}

void DxCommand::Initialize(const std::string& commandListKey, const std::string& commandQueueKey, D3D12_COMMAND_LIST_TYPE listType) {
    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::GetInstance()->GetDxDevice()->device_;

    // キーで指定したコマンドリストとコマンドキューを取得または生成
    commandListComboKey_ = commandListKey;
    commandQueueKey_     = commandQueueKey;

    LOG_DEBUG("Initialize DxCommand \n CommandList  :{} \n CommandQueue : {}", commandListComboKey_, commandQueueKey_);

    /*-----見つからなかった場合-----*/
    if (commandQueueMap_.count(commandQueueKey_) == 0) {
        ///================================================
        ///	CommandQueue の生成
        ///================================================
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        CreateCommandQueue(device, commandQueueKey_, commandQueueDesc);
    }
    commandQueue_ = commandQueueMap_[commandQueueKey_];

    /*-----見つからなかった場合-----*/
    if (commandListComboMap_.count(commandListComboKey_) == 0) {
        ///================================================
        ///	CommandList & CommandAllocator の生成
        ///================================================
        CreateCommandListWithAllocator(device, commandListComboKey_, listType);
    }

    // 取得
    auto& commandListCombo = commandListComboMap_[commandListComboKey_];
    commandList_           = commandListCombo.commandList;
    commandAllocator_      = commandListCombo.commandAllocator;
    resourceStateTracker_  = &commandListCombo.resourceStateTracker;
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

    // map にあるコンボのフラグを false にする
    auto it = commandListComboMap_.find(commandListComboKey_);
    if (it != commandListComboMap_.end()) {
        it->second.isClosed = false;
    }
}

void DxCommand::ResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter) {
    // リソースのバリアを設定
    // resourceStateTracker_ で リソースの状態を管理
    if (resourceStateTracker_) {
        resourceStateTracker_->Barrier(commandList_.Get(), resource.Get(), stateAfter);
    } else {
        LOG_CRITICAL("ResourceStateTracker is not initialized.");
    }
}

void DxCommand::ResourceDirectBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_BARRIER barrier) {
    // リソースのバリアをユーザーが作成したバリアで設定
    // resourceStateTracker_ で リソースの状態を管理

    if (resourceStateTracker_) {
        resourceStateTracker_->DirectBarrier(commandList_.Get(), resource.Get(), barrier);
    } else {
        LOG_CRITICAL("ResourceStateTracker is not initialized.");
    }
}

HRESULT DxCommand::Close() {
    HRESULT hr = commandList_->Close();

    if (FAILED(hr)) {
        OutputDebugStringA("CommandList Close FAILED! ptr=");
        char buf[64];
        sprintf_s(buf, "%p", static_cast<void*>(commandList_.Get()));

        OutputDebugStringA(buf);
        // 可能なら D3D12/DXGI エラー文字列も出す
        if (commandList_) {
            OutputDebugStringW(ConvertString(std::format("{} : {}", commandListComboKey_, HrToString(hr))).c_str());
        }
    } else {
        // close成功
        auto itr = commandListComboMap_.find(commandListComboKey_);
        if (itr != commandListComboMap_.end()) {
            itr->second.isClosed = true;
        }
    }

    return hr;
}

void DxCommand::ExecuteCommand() {
    // close 確認
    auto itr = commandListComboMap_.find(commandListComboKey_);
    if (itr != commandListComboMap_.end()) {
        if (!itr->second.isClosed) {
            LOG_ERROR("CommandList is not closed. Key: {}", commandListComboKey_);
            assert(false);
            return;
        }
    } else {
        LOG_ERROR("CommandListCombo not found. Key: {}", commandListComboKey_);
        assert(false);
        return;
    }

    // 実行
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

void DxCommand::ClearTarget(const DxRtvDescriptor& _rtv, const DxDsvDescriptor& _dsv, const Vec4f& _clearColor) {

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle           = _dsv.GetCpuHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtvHandle = _rtv.GetCpuHandle();

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
        "Finalize DxCommand \n CommandList Key : {} \n CommandQueue Key :{}", commandListComboKey_, commandQueueKey_);

    ///=====================================================
    // それぞれの ComPtr の参照カウントを確認
    ///=====================================================
    ULONG listRefCount      = GetComRefCount(commandList_);
    ULONG allocatorRefCount = GetComRefCount(commandAllocator_);
    ULONG queueRefCount     = GetComRefCount(commandQueue_);

    LOG_DEBUG("CommandList      Name {} RefCount : {}", commandListComboKey_, listRefCount);
    LOG_DEBUG("CommandAllocator Name {} RefCount : {}", commandListComboKey_, allocatorRefCount);
    LOG_DEBUG("CommandQueue     Name {} RefCount : {}", commandQueueKey_, queueRefCount);

    ///=====================================================
    // それぞれの ComPtr の参照カウントを確認して削除 (＝＝ 2 なのは this + static)
    ///=====================================================
    if (commandList_ && GetComRefCount(commandList_) == 2) {
        LOG_DEBUG("Delete CommandList : {}", commandListComboKey_);
        commandListComboMap_.erase(commandListComboKey_);
        resourceStateTracker_ = nullptr;
    }

    if (commandQueue_ && GetComRefCount(commandQueue_) == 2) {
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
        auto& list      = listCombo.commandList;
        auto& allocator = listCombo.commandAllocator;
        list.Reset();
        allocator.Reset();
    }
    for (auto& queue : commandQueueMap_) {
        queue.second.Reset();
    }
}
