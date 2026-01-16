#include "ResourceStateTracker.h"

/// util
#include "logger/Logger.h"

namespace OriGine {

std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStateTracker::globalResourceStates_;

void ResourceStateTracker::RegisterResource(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _initialState) {
    globalResourceStates_[_resource] = _initialState;
}

void ResourceStateTracker::UnregisterResource(ID3D12Resource* _resource) {
    if (_resource == nullptr) {
        return;
    }
    globalResourceStates_.erase(_resource);
}

void ResourceStateTracker::ClearGlobalResourceStates() {
    globalResourceStates_.clear();
}

void ResourceStateTracker::RegisterResource2Local(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _initialState) {
    // ローカルとグローバルの両方に登録
    localResourceStates_[_resource]  = _initialState;
    globalResourceStates_[_resource] = _initialState;
}

void ResourceStateTracker::Barrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ID3D12Resource* _resource, D3D12_RESOURCE_STATES _stateAfter) {
    // まずローカルを参照
    D3D12_RESOURCE_STATES stateBefore;
    auto it = localResourceStates_.find(_resource);
    if (it == localResourceStates_.end()) {
        // ローカルになければグローバル状態から初期化
        stateBefore                     = globalResourceStates_[_resource];
        localResourceStates_[_resource] = stateBefore;
    } else {
        stateBefore = it->second;
    }

    // 状態が同じならバリア不要
    if (stateBefore == _stateAfter) {
        return;
    }
    // ローカルになければ グローバル状態から初期化
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = _resource;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter  = _stateAfter;

    // ローカル状態を更新
    localResourceStates_[_resource] = _stateAfter;

    // バリア発行
    _commandList->ResourceBarrier(1, &barrier);
}

void ResourceStateTracker::DirectBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, ID3D12Resource* _resource, D3D12_RESOURCE_BARRIER _barrier) {
    localResourceStates_[_resource] = _barrier.Transition.StateAfter;

    _commandList->ResourceBarrier(1, &_barrier);
}

void ResourceStateTracker::CommitLocalStatesToGlobal() {
    for (const auto& [resource, state] : localResourceStates_) {
        globalResourceStates_[resource] = state;
    }
    localResourceStates_.clear();
}

} // namespace OriGine
