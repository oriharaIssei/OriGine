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
#include "component/material/light/lightManager.h"
#include "component/material/Material.h"
#include "input/Input.h"
#include "winApp/WinApp.h"
// DirectX Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxDsv.h"
#include "directX12/DxFence.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxSwapChain.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
// component
// #include "object3d/Object3d.h"
// etc
#include "deltaTime/DeltaTime.h"
// math
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Engine {
    friend class PrimitiveDrawer;

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
    Engine() = default;
    ~Engine() {};
    Engine(const Engine&)                  = delete;
    const Engine& operator=(const Engine&) = delete;

private:
    // api
    std::unique_ptr<WinApp> window_;
    Input* input_;

    // directX
    std::unique_ptr<DxDevice> dxDevice_;
    std::unique_ptr<DxCommand> dxCommand_;
    std::unique_ptr<DxSwapChain> dxSwapChain_;
    std::unique_ptr<DxFence> dxFence_;
    std::unique_ptr<DxDsv> dxDsv_;

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

    DxDsv* getDsv() const { return dxDsv_.get(); }

    float getDeltaTime() const { return deltaTime_->getDeltaTime(); }

    LightManager* getLightManager() const { return lightManager_; }
};
