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
#include "deltaTime/DeltaTime.h"
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
/// エンジン本体
/// </summary>
class Engine {
public:
    static Engine* GetInstance();

public:
    void Initialize();
    void Finalize();
    /// <summary>
    /// ウィンドウメッセージ処理
    /// </summary>
    /// <returns></returns>
    bool ProcessMessage();

    /// <summary>
    /// フレーム開始処理
    /// </summary>
    void BeginFrame();
    /// <summary>
    /// フレーム終了処理
    /// </summary>
    void EndFrame();

    /// <summary>
    /// 画面描画前処理
    /// </summary>
    void ScreenPreDraw();
    /// <summary>
    /// 画面描画後処理
    /// </summary>
    void ScreenPostDraw();

private:
    Engine();
    ~Engine();
    Engine(const Engine&)                  = delete;
    const Engine& operator=(const Engine&) = delete;

    void CreateDsv();

private:
    // api
    ::std::unique_ptr<WinApp> window_;
    InputManager* input_;

    // directX
    ::std::unique_ptr<DxDevice> dxDevice_;
    ::std::unique_ptr<DxCommand> dxCommand_;
    ::std::unique_ptr<DxSwapChain> dxSwapChain_;
    DxResource dsvResource_;
    DxDsvDescriptor dxDsv_;
    ::std::unique_ptr<DxFence> dxFence_;
    // resource
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::RTV>> rtvHeap_;
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>> srvHeap_;
    ::std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::DSV>> dsvHeap_;

    LightManager* lightManager_ = nullptr;
    // Time
    ::std::unique_ptr<DeltaTime> deltaTime_;

    ::std::unordered_map<int32_t, int32_t> resizeEventConvertIndexMap_;
    ::std::vector<::std::function<void(const Vec2f&)>> windowResizeEvents_;

public:
    WinApp* GetWinApp() { return window_.get(); }

    DxDevice* GetDxDevice() const { return dxDevice_.get(); }
    DxCommand* GetDxCommand() const { return dxCommand_.get(); }
    DxSwapChain* GetDxSwapChain() const { return dxSwapChain_.get(); }
    DxFence* GetDxFence() const { return dxFence_.get(); }

    DxDsvDescriptor& GetDxDsv() { return dxDsv_; }
    DxResource* GetDsvResource() { return &dsvResource_; }

    DxDescriptorHeap<DxDescriptorHeapType::RTV>* GetRtvHeap() const { return rtvHeap_.get(); }
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* GetSrvHeap() const { return srvHeap_.get(); }
    DxDescriptorHeap<DxDescriptorHeapType::DSV>* GetDsvHeap() const { return dsvHeap_.get(); }

    float GetDeltaTime() const { return deltaTime_->GetDeltaTime(); }
    void SetDeltaTime(float dt) { deltaTime_->SetDeltaTime(dt); }

    LightManager* GetLightManager() const { return lightManager_; }

    /// <summary>
    /// WindowResize時に呼ばれるイベントを登録する
    /// </summary>
    /// <param name="event">イベント関数</param>
    /// <returns>イベントのインデックス</returns>
    int32_t AddWindowResizeEvent(const ::std::function<void(const Vec2f&)>& _event) {
        windowResizeEvents_.push_back(_event);
        return static_cast<int32_t>(windowResizeEvents_.size() - 1);
    }

    void RemoveWindowResizeEvent(int32_t index) {
        if (index < 0 || index >= static_cast<int32_t>(windowResizeEvents_.size())) {
            LOG_WARN("Invalid window resize event index: {}", index);
            return;
        }
        windowResizeEvents_.erase(windowResizeEvents_.begin() + index);
    }
};

} // namespace OriGine
