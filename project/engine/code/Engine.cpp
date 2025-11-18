#include "Engine.h"
/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/DxSwapChain.h"

// module
#include "camera/CameraManager.h"
#include "component/animation/AnimationManager.h"
#include "component/material/light/LightManager.h"
#include "imGuiManager/ImGuiManager.h"
#include "input/InputManager.h"
#include "model/ModelManager.h"
#include "scene/SceneManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"

// asSets
#include "Audio/Audio.h"

// dx12Object
#include "directX12/DxFunctionHelper.h"
#include "directX12/RenderTexture.h"

#include "logger/Logger.h"

/// util
#include "util/StringUtil.h"

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

Engine* Engine::GetInstance() {
    static Engine instance;
    return &instance;
}

Engine::Engine() {}

Engine::~Engine() {}

void Engine::CreateDsv() {
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = UINT64(window_->GetWidth());
    resourceDesc.Height           = UINT64(window_->GetHeight());
    resourceDesc.MipLevels        = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // heap の設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 最大値でクリア
    depthClearValue.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT; // Resource と合わせる

    HRESULT result = dxDevice_->device_->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(dsvResource_.GetResourceRef().GetAddressOf()));

    if (FAILED(result)) {
        // エラーログを出力
        LOG_ERROR("Failed to create depth stencil view resource.");
        assert(false);
    }

    // DSV の設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT; // resourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2d Texture

    dxDsv_ = dsvHeap_->CreateDescriptor(dsvDesc, &dsvResource_);
}

void Engine::Initialize() {
    window_ = std::make_unique<WinApp>();

    SerializedField<std::string> windowTitle{"Settings", "Window", "Title"};
    SerializedField<Vec2f> windowSize{"Settings", "Window", "Size"};

    UINT windowStyle = 0;

#ifdef _DEBUG
    windowStyle = WS_OVERLAPPEDWINDOW;
#else
    windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
#endif // DEBUG

    window_->CreateGameWindow(ConvertString(windowTitle).c_str(), windowStyle, int32_t(windowSize->v[X]), int32_t(windowSize->v[Y]));

    input_ = InputManager::GetInstance();
    input_->Initialize(window_->GetHwnd());

    Audio::StaticInitialize();

    dxDevice_ = std::make_unique<DxDevice>();
    dxDevice_->Initialize();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    srvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>>(512);
    srvHeap_->Initialize(dxDevice_->device_);
    rtvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::RTV>>(24);
    rtvHeap_->Initialize(dxDevice_->device_);
    dsvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::DSV>>(8);
    dsvHeap_->Initialize(dxDevice_->device_);

    dxSwapChain_ = std::make_unique<DxSwapChain>();
    dxSwapChain_->Initialize(window_.get(), dxDevice_.get(), dxCommand_.get());

    dxFence_ = std::make_unique<DxFence>();
    dxFence_->Initialize(dxDevice_->device_);

    CreateDsv();

    ShaderManager::GetInstance()->Initialize();

    ImGuiManager::GetInstance()->Initialize(window_.get(), dxDevice_.get(), dxSwapChain_.get());

    TextureManager::Initialize();

    lightManager_ = LightManager::GetInstance();
    lightManager_->Initialize();

    ModelManager::GetInstance()->Initialize();
    RenderTexture::Awake();

    deltaTime_ = std::make_unique<DeltaTime>();
    deltaTime_->Initialize();

    AnimationManager::GetInstance()->Initialize();
    CameraManager::GetInstance()->Initialize();
}

void Engine::Finalize() {

    AnimationManager::GetInstance()->Finalize();
    CameraManager::GetInstance()->Finalize();
    lightManager_->Finalize();

#ifdef _DEBUG
    ImGuiManager::GetInstance()->Finalize();
#endif // _DEBUG
    ShaderManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    TextureManager::Finalize();

    dsvResource_.Finalize();
    dsvHeap_->ReleaseDescriptor(dxDsv_);

    dxSwapChain_->Finalize();
    dxCommand_->Finalize();
    DxCommand::ResetAll();
    dxFence_->Finalize();

    dsvHeap_->Finalize();
    rtvHeap_->Finalize();
    srvHeap_->Finalize();

    dxDevice_->Finalize();

    input_->Finalize();
    Audio::StaticFinalize();

    ResourceStateTracker::ClearGlobalResourceStates();
}

bool Engine::ProcessMessage() {
    return window_->ProcessMessage();
}

constexpr float MAX_DELTATIME = 1.f / 30.f;
void Engine::BeginFrame() {
    deltaTime_->Update();
    if (deltaTime_->GetDeltaTime() > MAX_DELTATIME) {
        deltaTime_->SetDeltaTime(MAX_DELTATIME);
    }

    window_->UpdateActivity();

#ifndef _DEBUG
    if (!window_->IsActive()) {
        return;
    }
#endif // !_DEBUG

    if (window_->isReSized()) {
        // ウィンドウのサイズ変更時の処理
        LOG_INFO("Window resized to: {}x{}", window_->GetWidth(), window_->GetHeight());

        UINT width  = window_->GetWidth();
        UINT height = window_->GetHeight();

        // GPU の同期を確保
        dxFence_->Signal(dxCommand_->GetCommandQueue());
        dxFence_->WaitForFence();

        dxSwapChain_->ResizeBuffer(width, height);

        dsvResource_.Finalize();
        dsvHeap_->ReleaseDescriptor(dxDsv_);
        CreateDsv();

        for (auto& event : windowResizeEvents_) {
            event(Vec2f{float(width), float(height)});
        }

        window_->SetIsReSized(false);
    }

    ImGuiManager::GetInstance()->Begin();

    input_->Update();

    lightManager_->Update();
}

void Engine::EndFrame() {
    ImGuiManager::GetInstance()->End();
}

void Engine::ScreenPreDraw() {
    DxFH::PreDraw(dxCommand_.get(), window_.get(), dxDsv_, dxSwapChain_.get());
}

void Engine::ScreenPostDraw() {
    ImGuiManager::GetInstance()->Draw();

    ///===============================================================
    ///	バリアの更新(描画->表示状態)
    ///===============================================================
    dxCommand_->ResourceBarrier(
        dxSwapChain_->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT);
    ///===============================================================

    // コマンドの受付終了 -----------------------------------
    HRESULT result = dxCommand_->Close();
    if (FAILED(result)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(result));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand_->ExecuteCommand();
    ///===============================================================

    dxSwapChain_->Present();

    // Frame Lock
    /*deltaTime_->Update();
    while(deltaTime_->GetDeltaTime() >= 1.0f / fps_){
        deltaTime_->Update();
    }*/
    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    dxFence_->Signal(dxCommand_->GetCommandQueue());
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
