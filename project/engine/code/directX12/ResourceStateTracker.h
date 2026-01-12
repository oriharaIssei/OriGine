#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <unordered_map>

namespace OriGine {

/// <summary>
/// DirectX 12 における各リソースの現在の状態 (D3D12_RESOURCE_STATES) を追跡・管理し、
/// 必要なタイミングで効率的に Resource Barrier を発行するためのクラス.
/// </summary>
class ResourceStateTracker {
public:
    /// <summary>
    /// アプリケーション全体で共有されるグローバルなリソース状態管理テーブルにリソースを登録する.
    /// </summary>
    /// <param name="resource">登録するリソース</param>
    /// <param name="initialState">初期状態</param>
    static void RegisterResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState);

    /// <summary>
    /// 管理テーブルからリソースを削除する. リソース破棄時に呼び出す必要がある.
    /// </summary>
    /// <param name="resource">削除するリソース</param>
    static void UnregisterResource(ID3D12Resource* resource);

    /// <summary>
    /// すべてのグローバルリソース状態の追跡をリセットする.
    /// </summary>
    static void ClearGlobalResourceStates();

private:
    /// <summary>すべての既存リソースの現在の状態を保持する静的マップ</summary>
    static ::std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> globalResourceStates_;

public:
    ResourceStateTracker()  = default;
    ~ResourceStateTracker() = default;

    /// <summary>
    /// コマンドリスト内での局所的なリソース状態追跡テーブルにリソースを登録する.
    /// </summary>
    /// <param name="resource">リソース</param>
    /// <param name="initialState">現在の推定状態</param>
    void RegisterResource2Local(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState);

    /// <summary>
    /// リソースを指定の状態へ遷移させるための Resource Barrier をコマンドリストに積む.
    /// 現在の状態が未知の場合は Pending Barrier として保持し、後で解決する.
    /// </summary>
    /// <param name="commandList">バリアを積み込むコマンドリスト</param>
    /// <param name="resource">バリアを張る対象のリソース</param>
    /// <param name="stateAfter">遷移先の状態</param>
    void Barrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter);

    /// <summary>
    /// 既に構築済みの D3D12_RESOURCE_BARRIER 構造体を使用して直接バリアを発行する.
    /// </summary>
    /// <param name="commandList">コマンドリスト</param>
    /// <param name="resource">リソース</param>
    /// <param name="barrier">設定済みバリア構造体</param>
    void DirectBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_BARRIER barrier);

    /// <summary>
    /// コマンドリストの実行直前などに呼び出し、このトラッカーが追跡していたローカルなリソース状態を
    /// 全体共有のグローバル状態管理テーブルへ統合する.
    /// </summary>
    void CommitLocalStatesToGlobal();

private:
    /// <summary>現在のコマンドリストに関連付けられた局所的なリソース状態の記録</summary>
    ::std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> localResourceStates_;
};

} // namespace OriGine
