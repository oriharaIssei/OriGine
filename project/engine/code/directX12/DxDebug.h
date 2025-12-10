#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

namespace OriGine {

/// <summary>
/// DirectX12 をデバッグするためのクラス. デストラクタ時にReportLiveObjectsを呼び出す.
/// </summary>
class DxDebug {
public:
    static DxDebug* GetInstance() {
        static DxDebug instance;
        return &instance;
    }

    /// <summary>
    /// デバッグレイヤーの有効化等の初期化処理
    /// </summary>
    void InitializeDebugger();
    /// <summary>
    /// 終了処理. ReportLiveObjectsを呼び出す.
    /// </summary>
    void FinalizeDebugger();

    /// <summary>
    /// ID3D12InfoQueueの作成. デバッグレイヤーが有効化されていないと失敗する.
    /// </summary>
    void CreateInfoQueue();

private:
    DxDebug();
    ~DxDebug();
    DxDebug(const DxDebug&)            = delete;
    DxDebug& operator=(const DxDebug&) = delete;

private:
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_    = nullptr;

public:
    const Microsoft::WRL::ComPtr<ID3D12Debug1>& GetDebugController() const {
        return debugController_;
    }
    Microsoft::WRL::ComPtr<ID3D12Debug1> GetDebugControllerRef() {
        return debugController_;
    }
    const Microsoft::WRL::ComPtr<ID3D12InfoQueue>& GetInfoQueue() const {
        return infoQueue_;
    }
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> GetInfoQueueRef() {
        return infoQueue_;
    }

    void SetDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity);
};

} // namespace OriGine
