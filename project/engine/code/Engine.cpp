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
#include "component/collision/collider/base/CollisionCategoryManager.h"
#include "component/material/light/LightManager.h"
#include "imGuiManager/ImGuiManager.h"
#include "input/InputManager.h"
#include "model/ModelManager.h"
#include "scene/SceneManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"

// assets
#include "Audio/Audio.h"

// dx12Object
#include "directX12/DxFunctionHelper.h"
#include "directX12/RenderTexture.h"
#include "directX12/ResourceStateTracker.h"

#include "logger/Logger.h"

#include "EngineConfig.h"

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

using namespace OriGine;

/// <summary> シングルトンインスタンスを取得する. </summary>
Engine* Engine::GetInstance() {
    static Engine instance;
    return &instance;
}

Engine::Engine() {}

Engine::~Engine() {}

/// <summary> 深度ステンシルバッファ（DSV）の生成. ウィンドウサイズに合わせてバッファを構築する. </summary>
void Engine::CreateDsv() {
    // DSVリソースの作成（解像度はウィンドウサイズに依存）
    dsvResource_.CreateDSVBuffer(dxDevice_->device_, static_cast<UINT64>(window_->GetWidth()), static_cast<UINT>(window_->GetHeight()));

    // DSV ビューの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24bit Depth / 8bit Stencil
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2D テクスチャとして扱う

    DSVEntry dsvEntry{&dsvResource_, dsvDesc};
    dxDsv_ = dsvHeap_->CreateDescriptor(&dsvEntry);
}

/// <summary>
/// エンジンの初期化処理.
/// ウィンドウの生成から DirectX12 関連の全コアオブジェクト、各種マネージャーのセットアップを行う.
/// </summary>
void Engine::Initialize() {
    window_ = std::make_unique<WinApp>();

    // 外部設定ファイルからウィンドウタイトルとサイズを読み込む
    SerializedField<std::string> windowTitle{"Settings", "Window", "Title"};
    SerializedField<Vec2f> windowSize{"Settings", "Window", "Size"};

    UINT windowStyle = 0;

#ifdef _DEBUG
    windowStyle = WS_OVERLAPPEDWINDOW; // デバッグ時はリサイズ可能なウィンドウ
#else
    windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU; // リリース時は固定サイズ
#endif // DEBUG

    window_->CreateGameWindow(ConvertString(windowTitle).c_str(), windowStyle, int32_t(windowSize->v[X]), int32_t(windowSize->v[Y]));

    // 入力システムの初期化
    input_ = InputManager::GetInstance();
    input_->Initialize(window_->GetHwnd());

    Audio::StaticInitialize();

    // DX12 デバイスの初期化
    dxDevice_ = std::make_unique<DxDevice>();
    dxDevice_->Initialize();

    // メインコマンドリストの初期化
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // グローバル記述子ヒープの作成
    srvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>>(Config::Rendering::kDefaultSrvHeapCount);
    srvHeap_->Initialize(dxDevice_->device_);
    rtvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::RTV>>(Config::Rendering::kDefaultRtvHeapCount);
    rtvHeap_->Initialize(dxDevice_->device_);
    dsvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::DSV>>(Config::Rendering::kDefaultDsvHeapCount);
    dsvHeap_->Initialize(dxDevice_->device_);

    // スワップチェーンの初期化
    dxSwapChain_ = std::make_unique<DxSwapChain>();
    dxSwapChain_->Initialize(window_.get(), dxDevice_.get(), dxCommand_.get());

    // 同期用フェンスの初期化
    dxFence_ = std::make_unique<DxFence>();
    dxFence_->Initialize(dxDevice_->device_);

    // 深度バッファの作成
    CreateDsv();

    // 各種エンジンスシステムの初期化
    ShaderManager::GetInstance()->Initialize();
    ImGuiManager::GetInstance()->Initialize(window_.get(), dxDevice_.get(), dxSwapChain_.get());
    TextureManager::Initialize();

    lightManager_ = LightManager::GetInstance();
    lightManager_->Initialize();

    ModelManager::GetInstance()->Initialize();
    RenderTexture::Awake();

    deltaTimer_ = std::make_unique<DeltaTimer>();
    deltaTimer_->Initialize();

    AnimationManager::GetInstance()->Initialize();
    CameraManager::GetInstance()->Initialize();

    auto* manager = OriGine::CollisionCategoryManager::GetInstance();
    manager->LoadFromGlobalVariables();
}

/// <summary> エンジンの終了処理. 各システムの Finalize を逆順に呼び出し、DX12 リソースを安全に解放する. </summary>
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

/// <summary> ウィンドウメッセージを処理する. </summary>
bool Engine::ProcessMessage() {
    return window_->ProcessMessage();
}

/// <summary> フレームの開始フェーズ. 経過時間の計算、ウィンドウリサイズ検知、入力更新を行う. </summary>
void Engine::BeginFrame() {
    deltaTimer_->Update();
    // デルタタイムが大きすぎる場合はキャップをかける（スパイク対策）
    if (deltaTimer_->GetDeltaTime() > Config::Time::kMaxDeltaTime) {
        deltaTimer_->SetDeltaTimer(Config::Time::kMaxDeltaTime);
    }

    window_->UpdateActivity();

#ifndef _DEBUG
    // 非アクティブ時は更新をスキップ
    if (!window_->IsActive()) {
        return;
    }
#endif // !_DEBUG

    // ウィンドウサイズ変更の検知とバックバッファの再構築
    if (window_->isReSized()) {
        LOG_INFO("Window resized to: {}x{}", window_->GetWidth(), window_->GetHeight());

        UINT width  = window_->GetWidth();
        UINT height = window_->GetHeight();

        // GPU の同期を確保してからバッファを再構築
        UINT64 fenceVal = dxFence_->Signal(dxCommand_->GetCommandQueue());
        dxFence_->WaitForFence(fenceVal);

        dxSwapChain_->ResizeBuffer(width, height);

        // 深度バッファも再構築
        dsvResource_.Finalize();
        dsvHeap_->ReleaseDescriptor(dxDsv_);
        CreateDsv();

        // 登録されているコールバックを実行
        for (auto& event : windowResizeEvents_) {
            event(Vec2f{float(width), float(height)});
        }

        window_->SetIsReSized(false);
    }

    ImGuiManager::GetInstance()->Begin();

    input_->Update();
    lightManager_->Update();
}

/// <summary> フレームの終了フェーズ（描画コマンド発行直前）. </summary>
void Engine::EndFrame() {
    ImGuiManager::GetInstance()->End();
}

/// <summary> 画面描画の準備. バックバッファのクリア等を行う. </summary>
void Engine::ScreenPreDraw() {
    DxFH::PreDraw(dxCommand_.get(), window_.get(), dxDsv_, dxSwapChain_.get());
}

/// <summary>
/// 画面描画の完了と表示.
/// ImGui の描画、リソースバリアの切り替え、コマンドの実行、スワップチェーンのフリップを行う.
/// </summary>
void Engine::ScreenPostDraw() {
    ImGuiManager::GetInstance()->Draw();

    // バックバッファを表示（PRESENT）状態に遷移
    dxCommand_->ResourceBarrier(
        dxSwapChain_->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT);

    // コマンドリストを閉じる
    HRESULT result = dxCommand_->Close();
    if (FAILED(result)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(result));
        assert(false);
    }

    // コマンドリストの実行
    dxCommand_->ExecuteCommand();

    // 表示の実行
    dxSwapChain_->Present();

    // GPU の完了待ち（簡易的な同期処理）
    UINT64 fenceVal = dxFence_->Signal(dxCommand_->GetCommandQueue());
    dxFence_->WaitForFence(fenceVal);

    // 次のフレームに向けてコマンドリストをリセット
    dxCommand_->CommandReset();
}
