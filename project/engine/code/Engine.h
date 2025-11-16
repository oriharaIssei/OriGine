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
struct DxDevice;
class DxFence;
class DxSwapChain;

#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

#include "deltaTime/DeltaTime.h"

class Engine {

public:
    static Engine* getInstance();

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
    WinApp* getWinApp() { return window_.get(); }

    DxDevice* getDxDevice() const { return dxDevice_.get(); }
    DxCommand* getDxCommand() const { return dxCommand_.get(); }
    DxSwapChain* getDxSwapChain() const { return dxSwapChain_.get(); }
    DxFence* getDxFence() const { return dxFence_.get(); }

    const DxDsvDescriptor& getDxDsv() const { return dxDsv_; }
    DxResource* getDsvResource() { return &dsvResource_; }

    DxDescriptorHeap<DxDescriptorHeapType::RTV>* getRtvHeap() const { return rtvHeap_.get(); }
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* getSrvHeap() const { return srvHeap_.get(); }

    float getDeltaTime() const { return deltaTime_->getDeltaTime(); }
    void setDeltaTime(float dt) { deltaTime_->setDeltaTime(dt); }

    LightManager* getLightManager() const { return lightManager_; }

    /// <summary>
    /// WindowResize時に呼ばれるイベントを登録する
    /// </summary>
    /// <param name="event">イベント関数</param>
    /// <returns>イベントのインデックス</returns>
    int32_t addWindowResizeEvent(const std::function<void(const Vec2f&)>& event) {
        windowResizeEvents_.push_back(event);
        return static_cast<int32_t>(windowResizeEvents_.size() - 1);
    }

    void removeWindowResizeEvent(int32_t index) {
        if (index < 0 || index >= static_cast<int32_t>(windowResizeEvents_.size())) {
            LOG_WARN("Invalid window resize event index: {}", index);
            return;
        }
        windowResizeEvents_.erase(windowResizeEvents_.begin() + index);
    }
};
