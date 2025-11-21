#include "ResourceStateTracker.h"

#include "logger/Logger.h"

std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStateTracker::globalResourceStates_;

void ResourceStateTracker::RegisterResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState) {
    globalResourceStates_[resource] = initialState;
}

void ResourceStateTracker::UnregisterResource(ID3D12Resource* resource) {
    if (resource == nullptr) {
        return;
    }
    globalResourceStates_.erase(resource);
}

void ResourceStateTracker::ClearGlobalResourceStates() {
    globalResourceStates_.clear();
}

void ResourceStateTracker::RegisterResource2Local(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState) {
    // ローカルとグローバルの両方に登録
    localResourceStates_[resource]  = initialState;
    globalResourceStates_[resource] = initialState;
}

void ResourceStateTracker::Barrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter) {
    // まずローカルを参照
    D3D12_RESOURCE_STATES stateBefore;
    auto it = localResourceStates_.find(resource);
    if (it == localResourceStates_.end()) {
        // ローカルになければグローバル状態から初期化
        stateBefore                    = globalResourceStates_[resource];
        localResourceStates_[resource] = stateBefore;
    } else {
        stateBefore = it->second;
    }

    // 状態が同じならバリア不要
    if (stateBefore == stateAfter) {
        return;
    }
    // ローカルになければ グローバル状態から初期化
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = resource;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter  = stateAfter;

    // ローカル状態を更新
    localResourceStates_[resource] = stateAfter;

    // バリア発行
    commandList->ResourceBarrier(1, &barrier);
}

void ResourceStateTracker::DirectBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_BARRIER barrier) {
    localResourceStates_[resource] = barrier.Transition.StateAfter;

    commandList->ResourceBarrier(1, &barrier);
}

void ResourceStateTracker::CommitLocalStatesToGlobal() {
    for (const auto& [resource, state] : localResourceStates_) {
        globalResourceStates_[resource] = state;
    }
    localResourceStates_.clear();
}
