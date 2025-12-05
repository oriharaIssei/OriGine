#pragma once

/// stl
// container
#include <array>
#include <vector>
// memory
#include <memory>
// string
#include <string>

/// engine
// module
class LightManager;
class InputManager;
class WinApp;
// DirectX Object
class DxCommand;
class DxDevice;
class DxFence;
class DxSwapChain;

#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

#include "deltaTime/DeltaTime.h"

namespace OriGine {

class Engine {

public:
    static Engine* GetInstance();

public:
    void Initialize();
    void Finalize();
    bool ProcessMessage();

    void BeginFrame();
    void EndFrame();

    void ScreenPreDraw();
    void ScreenPostDraw();

    int LoadTexture(const std::string& filePath);

private:
    Engine();
    ~Engine();
    Engine(const Engine&)                  = delete;
    const Engine& operator=(const Engine&) = delete;

    void CreateDsv();

private:
    // api
    std::unique_ptr<WinApp> window_;
    InputManager* input_;

    // directX
    std::unique_ptr<DxDevice> dxDevice_;
    std::unique_ptr<DxCommand> dxCommand_;
    std::unique_ptr<DxSwapChain> dxSwapChain_;
    DxResource dsvResource_;
    DxDsvDescriptor dxDsv_;
    std::unique_ptr<DxFence> dxFence_;
    // resource
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::RTV>> rtvHeap_;
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>> srvHeap_;
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::DSV>> dsvHeap_;

    LightManager* lightManager_ = nullptr;
    // Time
    std::unique_ptr<DeltaTime> deltaTime_;

    std::unordered_map<int32_t, int32_t> resizeEventConvertIndexMap_;
    std::vector<std::function<void(const Vec2f&)>> windowResizeEvents_;

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
    int32_t AddWindowResizeEvent(const std::function<void(const Vec2f&)>& event) {
        windowResizeEvents_.push_back(event);
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
