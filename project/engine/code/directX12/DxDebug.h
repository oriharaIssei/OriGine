#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

namespace OriGine {

/// <summary>
/// DirectX 12 のデバッグレイヤー、情報キュー (InfoQueue)、ライブオブジェクト報告などを管理するシングルトンクラス.
/// 開発時のリソースリーク検出や不正な API 呼び出しのデバッグに使用される.
/// </summary>
class DxDebug {
public:
    /// <summary>
    /// インスタンスを取得する.
    /// </summary>
    /// <returns>DxDebug インスタンス</returns>
    static DxDebug* GetInstance() {
        static DxDebug instance;
        return &instance;
    }

    /// <summary>
    /// デバッグレイヤーを有効化し、ファクトリのデバッグフラグを設定する等の初期化を行う.
    /// デバイス生成前に呼び出す必要がある.
    /// </summary>
    void InitializeDebugger();

    /// <summary>
    /// 終了処理を行い、アプリケーション終了時に ReportLiveObjects を呼び出してリソースリークをデバッグ出力に報告する.
    /// </summary>
    void FinalizeDebugger();

    /// <summary>
    /// デバイス生成後に呼び出し、特定の警告やエラーをフィルタリングするための ID3D12InfoQueue を作成する.
    /// デバッグレイヤーが有効でない場合は失敗する.
    /// </summary>
    void CreateInfoQueue();

private:
    DxDebug();
    ~DxDebug();
    DxDebug(const DxDebug&)            = delete;
    DxDebug& operator=(const DxDebug&) = delete;

private:
    /// <summary>デバッグ制御インターフェース</summary>
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
    /// <summary>デバッグメッセージフィルタリング用インターフェース</summary>
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_ = nullptr;

public:
    /// <summary>
    /// デバッグコントローラーを取得する.
    /// </summary>
    const Microsoft::WRL::ComPtr<ID3D12Debug1>& GetDebugController() const {
        return debugController_;
    }

    /// <summary>
    /// デバッグコントローラーの ComPtr をコピー取得する.
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12Debug1> GetDebugControllerRef() {
        return debugController_;
    }

    /// <summary>
    /// 情報キューを取得する.
    /// </summary>
    const Microsoft::WRL::ComPtr<ID3D12InfoQueue>& GetInfoQueue() const {
        return infoQueue_;
    }

    /// <summary>
    /// 情報キューの ComPtr をコピー取得する.
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> GetInfoQueueRef() {
        return infoQueue_;
    }

    /// <summary>
    /// 出力するデバッグメッセージの重要度の最小しきい値を設定する.
    /// </summary>
    /// <param name="severity">許容する最小重要度</param>
    void SetDebugMessageSeverity(D3D12_MESSAGE_SEVERITY severity);
};

} // namespace OriGine
