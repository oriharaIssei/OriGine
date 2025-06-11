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
class Input;
class WinApp;
// DirectX Object
class DxCommand;
class DxDevice;
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
    Input* input_;

    // directX
    std::unique_ptr<DxDevice> dxDevice_;
    std::unique_ptr<DxCommand> dxCommand_;
    std::unique_ptr<DxSwapChain> dxSwapChain_;
    DxResource dsvResource_;
    std::shared_ptr<DxDsvDescriptor> dxDsv_;
    std::unique_ptr<DxFence> dxFence_;
    // resource
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::RTV>> rtvHeap_;
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::SRV>> srvHeap_;
    std::unique_ptr<DxDescriptorHeap<DxDescriptorHeapType::DSV>> dsvHeap_;

    LightManager* lightManager_ = nullptr;
    // Time
    std::unique_ptr<DeltaTime> deltaTime_;
    float fps_ = 60.0f;

public:
    WinApp* getWinApp() { return window_.get(); }

    DxDevice* getDxDevice() const { return dxDevice_.get(); }
    DxCommand* getDxCommand() const { return dxCommand_.get(); }
    DxSwapChain* getDxSwapChain() const { return dxSwapChain_.get(); }
    DxFence* getDxFence() const { return dxFence_.get(); }

    DxDsvDescriptor* getDxDsv() const { return dxDsv_.get(); }
    DxResource* getDsvResource() { return &dsvResource_; }

    DxDescriptorHeap<DxDescriptorHeapType::RTV>* getRtvHeap() const { return rtvHeap_.get(); }
    DxDescriptorHeap<DxDescriptorHeapType::SRV>* getSrvHeap() const { return srvHeap_.get(); }
    DxDescriptorHeap<DxDescriptorHeapType::DSV>* getDsvHeap() const { return dsvHeap_.get(); }

    float getDeltaTime() const { return deltaTime_->getDeltaTime(); }

    LightManager* getLightManager() const { return lightManager_; }
};
