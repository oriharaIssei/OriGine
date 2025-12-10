#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <unordered_map>

namespace OriGine {

/// <summary>
/// リソースの状態遷移を管理するクラス
/// </summary>
class ResourceStateTracker {
public:
    /// <summary>
    /// グローバルリソース状態にリソースを登録する
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="initialState"></param>
    static void RegisterResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState);

    /// <summary>
    /// グローバルリソース状態からリソースを登録解除する
    /// </summary>
    static void UnregisterResource(ID3D12Resource* resource);

    /// <summary>
    /// グローバルリソース状態をクリアする
    /// </summary>
    static void ClearGlobalResourceStates();

private:
    static ::std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> globalResourceStates_;

public:
    ResourceStateTracker()  = default;
    ~ResourceStateTracker() = default;

    /// <summary>
    /// ローカルリソース状態にリソースを登録する
    /// </summary>
    void RegisterResource2Local(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState);

    /// <summary>
    /// リソースの状態をバリアを発行して遷移させる
    /// </summary>
    /// <param name="commandList">リソースにバリアを張るCommandList</param>
    /// <param name="resource">バリアを張られるリソース</param>
    /// <param name="stateAfter">次のResourceState</param>
    void Barrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter);

    /// <summary>
    /// 事前に作成したバリアを直接発行する
    /// </summary>
    void DirectBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_BARRIER barrier);

    /// <summary>
    /// ローカル状態をグローバル状態にコミットする
    /// </summary>
    void CommitLocalStatesToGlobal();

private:
    ::std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> localResourceStates_;
};

} // namespace OriGine
