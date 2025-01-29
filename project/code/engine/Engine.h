#pragma once

///stl
//container
#include <array>
#include <vector>
//memory
#include <memory>
//string
#include <string>

///engine
//module
#include "input/Input.h"
#include "material/Material.h"
#include "material/light/lightManager.h"
#include "winApp/WinApp.h"
#include "editor/EngineEditor.h"
//DirectX Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxDsv.h"
#include "directX12/DxFence.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxSwapChain.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
//component
#include "object3d/Object3d.h"
//etc
#include "deltaTime/DeltaTime.h"
#include "deltaTime/GameDeltaTime.h"
//math
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Engine{
    friend class PrimitiveDrawer;

public:
    static Engine* getInstance();

public:
    void Init();
    void Finalize();
    bool ProcessMessage();
    void BeginFrame();
    void EndFrame();

    void ScreenPreDraw();
    void ScreenPostDraw();

    int LoadTexture(const std::string& filePath);

private:
    Engine() = default;
    ~Engine(){};
    Engine(const Engine&)                  = delete;
    const Engine& operator=(const Engine&) = delete;

    void CreateTexturePSO();

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

    // PipelineState Object
    std::array<PipelineStateObj*,kBlendNum> texturePso_;
    std::array<std::string,kBlendNum> texturePsoKeys_;

    // module

#ifdef _DEBUG
    EngineEditor* editor_             = nullptr;
#endif // _DEBUG
    LightManager* lightManager_       = nullptr;
    std::unique_ptr<MaterialManager> materialManager_ = nullptr;
    // Time
    std::unique_ptr<DeltaTime> deltaTime_;
    float fps_ = 60.0f;

public:
    WinApp* getWinApp(){ return window_.get(); }

    DxDevice* getDxDevice() const{ return dxDevice_.get(); }
    DxCommand* getDxCommand() const{ return dxCommand_.get(); }
    DxSwapChain* getDxSwapChain() const{ return dxSwapChain_.get(); }
    DxFence* getDxFence() const{ return dxFence_.get(); }

    DxDsv* getDsv() const{ return dxDsv_.get(); }

    MaterialManager* getMaterialManager() const{ return materialManager_.get(); }

    PipelineStateObj* getTexturePso(BlendMode blend) const{ return texturePso_[static_cast<size_t>(blend)]; }

    const std::array<std::string,kBlendNum>& getTexturePsoKeys() const{ return texturePsoKeys_; }

    float getDeltaTime() const{ return deltaTime_->getDeltaTime(); }
    DeltaTime* getDeltaTimeInstance() const { return deltaTime_.get(); }

    LightManager* getLightManager() const{ return lightManager_; }
};
