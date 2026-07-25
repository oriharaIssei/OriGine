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

/// <summary>
/// リソースを指定の状態へ遷移させるバリアを積む。既に同じ状態なら何もしない。
/// </summary>
/// <remarks>
/// DirectX12ではリソースの用途(描画先・シェーダ読み込み・コピー元など)ごとに状態があり、
/// 用途を変えるときは必ずバリアで遷移を宣言しなければならない。
/// 宣言した状態と実際の状態がずれるとデバイスリムーブや描画崩れになるため、
/// 現在の状態をこのクラスで追跡している。
///
/// 状態をローカルとグローバルの2段で持つのが要点。
/// コマンドリストへの記録時点ではまだGPUが実行しておらず、他のコマンドリストが
/// 同じリソースをどう変えるかも確定していない。そこで記録中はそのリスト内での
/// 状態遷移をローカルに閉じて追い、実行順が確定した時点で
/// CommitLocalStatesToGlobal()によりグローバルへ反映する。
/// </remarks>
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
    // 同一状態への遷移バリアはドライバに拒否され、デバッグレイヤーで警告が出る。
    // また無駄なバリアはGPUのパイプラインを不必要に区切って性能を落とす
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
    // ユーザー指定のバリアが示す遷移後状態をローカル状態として記録しておく
    localResourceStates_[_resource] = _barrier.Transition.StateAfter;

    _commandList->ResourceBarrier(1, &_barrier);
}

void ResourceStateTracker::CommitLocalStatesToGlobal() {
    // このコマンドリストで確定したローカル状態をグローバルへ反映し、
    // 次にこのリソースを参照するコマンドリストが正しい初期状態を参照できるようにする
    for (const auto& [resource, state] : localResourceStates_) {
        globalResourceStates_[resource] = state;
    }
    localResourceStates_.clear();
}

} // namespace OriGine
