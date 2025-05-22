#pragma once

#include <unordered_map>

#include <wrl.h>

#include <d3d12.h>

/// <summary>
/// リソースの状態遷移を管理するクラス
/// </summary>
class ResourceStateTracker {
public:
    static void RegisterReosurce(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState) {
        globalResourceStates_[resource] = initialState;
    }

private:
    static std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> globalResourceStates_;

public:
    ResourceStateTracker()  = default;
    ~ResourceStateTracker() = default;

    void RegisterReosurce2Local(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState) {
        localResourceStates_[resource]  = initialState;
        globalResourceStates_[resource] = initialState;
    }

    /// <summary>
    /// リソースの状態をバリアを発行して遷移させる
    /// </summary>
    /// <param name="commandList">リソースにバリアを張るCommandList</param>
    /// <param name="resource">バリアを張られるリソース</param>
    /// <param name="stateAfter">次のResourceState</param>
    void Barrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter);

    void DirectBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_BARRIER barrier);

    /// <summary>
    /// ローカル状態をグローバル状態にコミットする
    /// </summary>
    void CommitLocalStatesToGlobal();

private:
    std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> localResourceStates_;
};
