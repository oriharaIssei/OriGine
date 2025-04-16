#include "Engine.h"

/// engine
// module
#include "animation/AnimationManager.h"
#include "camera/CameraManager.h"
#include "component/material/light/LightManager.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "imGuiManager/ImGuiManager.h"
#include "model/ModelManager.h"
#include "texture/TextureManager.h"

#ifdef _DEBUG
#include "ECSEditor.h"
#endif // _DEBUG

// assets
#include "Audio/Audio.h"

// dx12Object
#include "directX12/DxFunctionHelper.h"
#include "directX12/DxHeap.h"
#include "directX12/RenderTexture.h"

// lib
#include "logger/Logger.h"

// math

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#define _USE_MATH_DEFINES
#include <cmath>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "XInput.lib")

Engine* Engine::getInstance() {
    static Engine instance;
    return &instance;
}

void Engine::Initialize() {
    window_ = std::make_unique<WinApp>();
    window_->CreateGameWindow(L"LE2A_07_OriharaIssei_", WS_OVERLAPPEDWINDOW, 1280, 720);

    input_ = Input::getInstance();
    input_->Initialize();

    Audio::StaticInitialize();

    dxDevice_ = std::make_unique<DxDevice>();
    dxDevice_->Initialize();

    DxHeap* dxHeap = DxHeap::getInstance();
    dxHeap->Initialize(dxDevice_->getDevice());

    DxSrvArrayManager::getInstance()->Initialize();
    DxRtvArrayManager::getInstance()->Initialize();

    dxDsv_ = std::make_unique<DxDsv>();
    dxDsv_->Initialize(dxDevice_->getDevice(), dxHeap->getDsvHeap(), window_->getWidth(), window_->getHeight());

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    dxSwapChain_ = std::make_unique<DxSwapChain>();
    dxSwapChain_->Initialize(window_.get(), dxDevice_.get(), dxCommand_.get());


    dxFence_ = std::make_unique<DxFence>();
    dxFence_->Initialize(dxDevice_->getDevice());

    ShaderManager::getInstance()->Initialize();

    ImGuiManager::getInstance()->Initialize(window_.get(), dxDevice_.get(), dxSwapChain_.get());

    TextureManager::Initialize();

    lightManager_ = LightManager::getInstance();
    lightManager_->Initialize();

    ModelManager::getInstance()->Initialize();
    RenderTexture::Awake();

    deltaTime_ = std::make_unique<DeltaTime>();
    deltaTime_->Initialize();

    AnimationManager::getInstance()->Initialize();
    CameraManager::getInstance()->Initialize();

}

void Engine::Finalize() {

    AnimationManager::getInstance()->Finalize();
    CameraManager::getInstance()->Finalize();
    lightManager_->Finalize();

#ifdef _DEBUG
    ImGuiManager::getInstance()->Finalize();
#endif // _DEBUG
    ShaderManager::getInstance()->Finalize();
    ModelManager::getInstance()->Finalize();
    TextureManager::Finalize();

    DxSrvArrayManager::getInstance()->Finalize();
    DxRtvArrayManager::getInstance()->Finalize();
    dxDsv_->Finalize();

    DxHeap::getInstance()->Finalize();
    dxSwapChain_->Finalize();
    dxCommand_->Finalize();
    DxCommand::ResetAll();
    dxFence_->Finalize();
    dxDevice_->Finalize();

    input_->Finalize();
    Audio::StaticFinalize();
}

bool Engine::ProcessMessage() {
    return window_->ProcessMessage();
}

void Engine::BeginFrame() {
    ImGuiManager::getInstance()->Begin();

    input_->Update();
    // Sprite::setBlendMode(BlendMode::Alpha);
    deltaTime_->Update();

    lightManager_->Update();
}

void Engine::EndFrame() {
    ImGuiManager::getInstance()->End();
}

void Engine::ScreenPreDraw() {
    DxFH::PreDraw(dxCommand_.get(), window_.get(), dxSwapChain_.get());
}

void Engine::ScreenPostDraw() {
    ImGuiManager::getInstance()->Draw();

    HRESULT hr;
    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();
    ///===============================================================
    ///	バリアの更新(描画->表示状態)
    ///===============================================================
    ResourceBarrierManager::Barrier(
        commandList,
        dxSwapChain_->getCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT);
    ///===============================================================

    // コマンドの受付終了 -----------------------------------
    hr = commandList->Close();
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    ID3D12CommandList* ppHeaps[] = {commandList};
    dxCommand_->getCommandQueue()->ExecuteCommandLists(1, ppHeaps);
    ///===============================================================

    dxSwapChain_->Present();

    // Frame Lock
    /*deltaTime_->Update();
    while(deltaTime_->getDeltaTime() >= 1.0f / fps_){
        deltaTime_->Update();
    }*/
    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    dxFence_->Signal(dxCommand_->getCommandQueue());
    dxFence_->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}

int Engine::LoadTexture(const std::string& filePath) {
    return TextureManager::LoadTexture(filePath);
}
