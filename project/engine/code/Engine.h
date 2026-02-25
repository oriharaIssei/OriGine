#pragma once

/// stl
#include <functional>
// container
#include <array>
#include <unordered_map>
#include <vector>
// memory
#include <memory>
// string
#include <string>

/// engine
// directX12
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

/// util
#include "deltaTime/DeltaTimer.h"
#include "logger/Logger.h"

/// math
#include <cstdint>
#include <Vector2.h>

namespace OriGine {

/// engine
// module
class LightManager;
class InputManager;
class WinApp;
// DirectX Object
class DxCommand;
struct DxDevice;
class DxFence;
class DxSwapChain;

/// <summary>
/// OriGine エンジンの核となる統括クラス (シングルトン).
/// ウィンドウの生成、DirectX 12 の初期化、フレームのライフサイクル管理、主要なマネージャークラスの保持を担当する.
/// </summary>
class Engine {
public:
    /// <summary> エンジンの唯一のインスタンスを取得する. </summary>
    static Engine* GetInstance();

public:
    /// <summary>
    /// エンジンを構成する各モジュール (Window, DirectX12, 各マネージャー) の初期化を行う.
    /// シーンマネージャー、テクスチャマネージャー、ライトマネージャー等のシステムがここで一括してセットアップされる.
    /// </summary>
    void Initialize();

    /// <summary>
    /// エンジンの終了処理を行い、すべてのリソースを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// OS からのメッセージを処理する.
    /// </summary>
    /// <returns>ゲームを継続する場合は true、終了（WM_QUIT 受信時など）する場合は false</returns>
    bool ProcessMessage();

    /// <summary>
    /// 1 フレームの開始処理を行う.
    /// デルタタイムの更新、ウィンドウリサイズ検知、入力デバイスの更新開始、ImGui フレームの開始などを行う.
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// 1 フレームの終了処理を行う. ImGui の録画終了など、描画コマンド発行直前の処理を行う.
    /// </summary>
    void EndFrame();

    /// <summary>
    /// 標準のレンダーターゲット（バックバッファ）への描画準備を開始する.
    /// ビューポート設定、カラーバッファと深度バッファのクリア等を行う.
    /// </summary>
    void ScreenPreDraw();

    /// <summary>
    /// 標準のレンダーターゲットへの描画を終了し、表示（Present）を行う.
    /// ImGui の描画コマンド発行、リソースバリアの切り替え、コマンド実行、GPU 同期が含まれる.
    /// </summary>
    void ScreenPostDraw();

private:
    Engine();
    ~Engine();
    Engine(const Engine&)                  = delete;
    const Engine& operator=(const Engine&) = delete;

    /// <summary>
    /// 標準のデプスステンシルバッファ（DSV）リソースとそのビューを生成する.
    /// ウィンドウサイズが変更された際にも呼び出される.
    /// </summary>
    void CreateDsv();

private:
    ::std::unique_ptr<WinApp> window_; // メインウィンドウ管理
    InputManager* input_; // 入力管理

    // --- DirectX 12 Core Objects ---
    ::std::unique_ptr<DxDevice> dxDevice_; // D3D12 デバイス
    ::std::unique_ptr<DxCommand> dxCommand_; // メインコマンド管理
    ::std::unique_ptr<DxSwapChain> dxSwapChain_; // スワップチェイン
    DxResource dsvResource_; // 深度バッファリソース
    DxDsvDescriptor dxDsv_; // 深度バッファビュー
    ::std::unique_ptr<DxFence> dxFence_; // 同期用フェンス

    // --- Global Descriptor Heaps ---
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::RTV>> rtvHeap_; // RTV ヒープ
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>> srvHeap_; // SRV ヒープ
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::DSV>> dsvHeap_; // DSV ヒープ

    LightManager* lightManager_ = nullptr; // ライト管理
    ::std::unique_ptr<DeltaTimer> deltaTimer_; // デルタタイマー

    ::std::unordered_map<int32_t, int32_t> resizeEventConvertIndexMap_;
    ::std::vector<::std::function<void(const Vec2f&)>> windowResizeEvents_; // ウィンドウリサイズ時のコールバックリスト

public:
    /// <summary> ウィンドウ管理オブジェクトを取得する. </summary>
    WinApp* GetWinApp() { return window_.get(); }

    /// <summary> D3D12 デバイス管理オブジェクトを取得する. </summary>
    DxDevice* GetDxDevice() const { return dxDevice_.get(); }
    /// <summary> メインコマンド管理オブジェクトを取得する. </summary>
    DxCommand* GetDxCommand() const { return dxCommand_.get(); }
    /// <summary> スワップチェイン管理オブジェクトを取得する. </summary>
    DxSwapChain* GetDxSwapChain() const { return dxSwapChain_.get(); }
    /// <summary> 同期用フェンスオブジェクトを取得する. </summary>
    DxFence* GetDxFence() const { return dxFence_.get(); }

    /// <summary> 標準のデプスステンシルビューを取得する. </summary>
    DxDsvDescriptor& GetDxDsv() { return dxDsv_; }
    /// <summary> 標準のデプスステンシルリソースを取得する. </summary>
    DxResource* GetDsvResource() { return &dsvResource_; }

    /// <summary> RTV 用グローバルヒープを取得する. </summary>
    DxDescriptorHeap<DxDescriptorHeapType::RTV>* GetRtvHeap() const { return rtvHeap_.get(); }
    /// <summary> SRV 用グローバルヒープを取得する. </summary>
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* GetSrvHeap() const { return srvHeap_.get(); }
    /// <summary> DSV 用グローバルヒープを取得する. </summary>
    DxDescriptorHeap<DxDescriptorHeapType::DSV>* GetDsvHeap() const { return dsvHeap_.get(); }

    /// <summary> デルタタイマーを取得する. </summary>
    DeltaTimer* GetDeltaTimer() { return deltaTimer_.get(); }
    /// <summary> 直前フレームからの経過時間（秒）を取得する. </summary>
    float GetDeltaTime() const { return deltaTimer_->GetDeltaTime(); }
    /// <summary> デルタ値を強制的に更新する. </summary>
    void SetDeltaTime(float _dt) { deltaTimer_->SetDeltaTime(_dt); }

    /// <summary> ライト管理オブジェクトを取得する. </summary>
    LightManager* GetLightManager() const { return lightManager_; }

    /// <summary>
    /// ウィンドウサイズ変更時に実行されるコールバック関数を登録する.
    /// </summary>
    /// <param name="_event">実行する関数（新しいサイズをVec2fで受け取る）</param>
    /// <returns>イベント ID（解除時に使用）</returns>
    int32_t AddWindowResizeEvent(const ::std::function<void(const Vec2f&)>& _event) {
        windowResizeEvents_.push_back(_event);
        return static_cast<int32_t>(windowResizeEvents_.size() - 1);
    }

    /// <summary>
    /// ウィンドウサイズ変更時に実行されるコールバック関数の登録を解除する.
    /// </summary>
    /// <param name="_index">登録時に返されたイベント ID</param>
    void RemoveWindowResizeEvent(int32_t _index) {
        if (_index < 0 || _index >= static_cast<int32_t>(windowResizeEvents_.size())) {
            LOG_WARN("Invalid window resize event index: {}", _index);
            return;
        }
        windowResizeEvents_.erase(windowResizeEvents_.begin() + _index);
    }
};

} // namespace OriGine
