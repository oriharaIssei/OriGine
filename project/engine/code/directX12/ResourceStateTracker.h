#pragma once

#include <unordered_map>

#include <wrl.h>

#include <d3d12.h>

/// <summary>
/// リソースの状態遷移を管理するクラス
/// </summary>
class ResourceStateTracker {
public:
    static void RegisterResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState) {
        globalResourceStates_[resource.Get()] = initialState;
    }

    static void UnregisterResource(ID3D12Resource* resource) {
        globalResourceStates_.erase(resource);
    }

    static void ClearGlobalResourceStates() {
        globalResourceStates_.clear();
    }

private:
    static std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> globalResourceStates_;

public:
    ResourceStateTracker()  = default;
    ~ResourceStateTracker() = default;

    void RegisterResource2Local(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState) {
        localResourceStates_[resource.Get()]  = initialState;
        globalResourceStates_[resource.Get()] = initialState;
    }

    /// <summary>
    /// リソースの状態をバリアを発行して遷移させる
    /// </summary>
    /// <param name="commandList">リソースにバリアを張るCommandList</param>
    /// <param name="resource">バリアを張られるリソース</param>
    /// <param name="stateAfter">次のResourceState</param>
    void Barrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter);

    void DirectBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_BARRIER barrier);

    /// <summary>
    /// ローカル状態をグローバル状態にコミットする
    /// </summary>
    void CommitLocalStatesToGlobal();

private:
    std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> localResourceStates_;
};
