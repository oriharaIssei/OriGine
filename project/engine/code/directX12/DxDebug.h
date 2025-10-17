#pragma once

#include <wrl.h>

#include <d3d12.h>

/// <summary>
/// DirectX12 をデバッグするためのクラス. デストラクタ時にReportLiveObjectsを呼び出す.
/// </summary>
class DxDebug {
public:
    static DxDebug* getInstance() {
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
    const Microsoft::WRL::ComPtr<ID3D12Debug1>& getDebugController() const {
        return debugController_;
    }
    Microsoft::WRL::ComPtr<ID3D12Debug1> getDebugControllerRef() {
        return debugController_;
    }
    const Microsoft::WRL::ComPtr<ID3D12InfoQueue>& getInfoQueue() const {
        return infoQueue_;
    }
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> getInfoQueueRef() {
        return infoQueue_;
    }

    void setDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity);
};
