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
#include "input/Input.h"
#include "model/ModelManager.h"
#include "sceneManager/SceneManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"

#ifdef _DEBUG
#include "ECSEditor.h"
#endif // _DEBUG

// assets
#include "Audio/Audio.h"

// dx12Object
#include "directX12/DxFunctionHelper.h"
#include "directX12/RenderTexture.h"

// lib
#include "logger/Logger.h"

/// util
#include "util/ConvertString.h"

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

Engine::Engine() {}

Engine::~Engine() {}

void Engine::CreateDsv() {
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = UINT64(window_->getWidth());
    resourceDesc.Height           = UINT64(window_->getHeight());
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

    HRESULT result = dxDevice_->getDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(dsvResource_.getResourceRef().GetAddressOf()));

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

    window_->CreateGameWindow(ConvertString(windowTitle).c_str(), WS_OVERLAPPEDWINDOW, int32_t(windowSize->v[X]), int32_t(windowSize->v[Y]));

    input_ = Input::getInstance();
    input_->Initialize();

    Audio::StaticInitialize();

    dxDevice_ = std::make_unique<DxDevice>();
    dxDevice_->Initialize();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    srvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::SRV>>(1024);
    srvHeap_->Initialize(dxDevice_->getDevice());
    rtvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::RTV>>(1024);
    rtvHeap_->Initialize(dxDevice_->getDevice());
    dsvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::DSV>>(16);
    dsvHeap_->Initialize(dxDevice_->getDevice());

    dxSwapChain_ = std::make_unique<DxSwapChain>();
    dxSwapChain_->Initialize(window_.get(), dxDevice_.get(), dxCommand_.get());

    dxFence_ = std::make_unique<DxFence>();
    dxFence_->Initialize(dxDevice_->getDevice());

    CreateDsv();

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

    dsvResource_.Finalize();

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

void Engine::BeginFrame() {
    if (window_->isReSized()) {
        UINT width  = window_->getWidth();
        UINT height = window_->getHeight();

        // GPU の同期を確保
        dxFence_->Signal(dxCommand_->getCommandQueue());
        dxFence_->WaitForFence();

        dxSwapChain_->ResizeBuffer(width, height);
        // dxDsv_->Resize(dxDevice_->getDevice(), DxHeap::getInstance()->getDsvHeap(), width, height);

        SceneManager::getInstance()->getSceneView()->Resize(window_->getWindowSize());

        window_->setIsReSized(false);
    }

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
    DxFH::PreDraw(dxCommand_.get(), window_.get(), dxDsv_.get(), dxSwapChain_.get());
}

void Engine::ScreenPostDraw() {
    ImGuiManager::getInstance()->Draw();

    ///===============================================================
    ///	バリアの更新(描画->表示状態)
    ///===============================================================
    dxCommand_->ResourceBarrier(
        dxSwapChain_->getCurrentBackBuffer().Get(),
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
