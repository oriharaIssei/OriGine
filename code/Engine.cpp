#include "Engine.h"

/// engine
#include "asset/AssetSystem.h"

// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/DxSwapChain.h"

// module
#include "asset/AssetSystem.h"
#include "camera/CameraManager.h"
#include "component/animation/AnimationManager.h"
#include "component/collision/collider/base/CollisionCategoryManager.h"
#include "component/material/light/LightManager.h"
#include "imGuiManager/ImGuiManager.h"
#include "input/InputManager.h"
#include "model/ModelManager.h"
#include "scene/SceneManager.h"
#include "text/FontManager.h"
#include "winApp/WinApp.h"

// messageBus
#include "messageBus/MessageBus.h"

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
    SerializedField<std::string> windowTitle{"Settings", "Window", "Title", "OriGine Application"};
    SerializedField<Vec2f> windowSize{"Settings", "Window", "Size", Vec2f(float(Config::Window::kDefaultClientWidth), float(Config::Window::kDefaultClientHeight))};

    UINT windowStyle = 0;

#ifdef _DEBUG
    windowStyle = WS_OVERLAPPEDWINDOW; // デバッグ時はリサイズ可能なウィンドウ
#else
    windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU; // リリース時は固定サイズ
#endif // DEBUG

    // ウィンドウハンドル(HWND)がこの後の DirectInput / DX12 / ImGui 等の初期化すべてで必要になるため、
    // 最初にウィンドウを生成しておく
    window_->CreateGameWindow(ConvertString(windowTitle).c_str(), windowStyle, int32_t(windowSize->v[X]), int32_t(windowSize->v[Y]));

    // 入力システムの初期化（ウィンドウハンドルに DirectInput を紐付けるため、ウィンドウ生成の直後に行う）
    input_ = InputManager::GetInstance();
    input_->Initialize(window_->GetHwnd());

    // オーディオエンジンは他のDX12初期化と依存関係がないため、順序に強い制約はない
    Audio::StaticInitialize();

    // DX12 デバイスの初期化（以降の DX12 関連オブジェクトはすべてこのデバイスに依存するため最初に必要）
    dxDevice_ = std::make_unique<DxDevice>();
    dxDevice_->Initialize();

    // メインコマンドリストの初期化（デバイス生成後でなければコマンドアロケータ/リストを作成できない）
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // グローバル記述子ヒープの作成
    // スワップチェーンの RTV や DSV の生成が、それぞれのヒープからディスクリプタを確保することに依存するため、
    // スワップチェーン・深度バッファの初期化より前に用意しておく
    srvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>>(Config::Rendering::kDefaultSrvHeapCount);
    srvHeap_->Initialize(dxDevice_->device_);
    rtvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::RTV>>(Config::Rendering::kDefaultRtvHeapCount);
    rtvHeap_->Initialize(dxDevice_->device_);
    dsvHeap_ = std::make_unique<DxDescriptorHeap<DxDescriptorHeapType::DSV>>(Config::Rendering::kDefaultDsvHeapCount);
    dsvHeap_->Initialize(dxDevice_->device_);

    // スワップチェーンの初期化（デバイス・コマンドキュー・RTVヒープが揃って初めて生成できる）
    dxSwapChain_ = std::make_unique<DxSwapChain>();
    dxSwapChain_->Initialize(window_.get(), dxDevice_.get(), dxCommand_.get());

    // 同期用フェンスの初期化（GPU/CPU間の同期はコマンドキュー生成後でないと意味を持たない）
    dxFence_ = std::make_unique<DxFence>();
    dxFence_->Initialize(dxDevice_->device_);

    // 深度バッファの作成（ウィンドウサイズ・DSVヒープの両方に依存するため、ここまでの初期化が終わってから行う）
    CreateDsv();

    // 各種エンジンスシステムの初期化
    // ここから先は DX12 のコアオブジェクト（デバイス・コマンド・ヒープ・スワップチェーン）に依存する
    // 上位システムの初期化フェーズであり、コアオブジェクトより後でなければ初期化できない
    ShaderManager::GetInstance()->Initialize();
    ImGuiManager::GetInstance()->Initialize(window_.get(), dxDevice_.get(), dxSwapChain_.get());

    lightManager_ = LightManager::GetInstance();
    lightManager_->Initialize();

    ModelManager::GetInstance()->Initialize();
    RenderTexture::Awake();

    FontManager::GetInstance()->Initialize();

    deltaTimer_ = std::make_unique<DeltaTimer>();
    deltaTimer_->Initialize();

    AnimationManager::GetInstance()->Initialize();
    CameraManager::GetInstance()->Initialize();

    // AssetSystem はテクスチャ等の読み込みに DX12 デバイス・コマンド・SRVヒープを使用するため、
    // それらの初期化がすべて完了した最後のタイミングで初期化する
    AssetSystem::GetInstance()->Initialize();

    auto* manager = OriGine::CollisionCategoryManager::GetInstance();
    manager->LoadFromGlobalVariables();
}

/// <summary> エンジンの終了処理. 各システムの Finalize を逆順に呼び出し、DX12 リソースを安全に解放する. </summary>
void Engine::Finalize() {

    // Initialize と逆順に破棄するのが基本方針。
    // AssetSystem はテクスチャ等の GPU リソース（SRV・DxResource）を保持しており、
    // それらの解放には DX12 デバイス・SRVヒープがまだ生きている必要があるため、
    // DX12 コアオブジェクトを壊す前に最初に終了させる。
    AssetSystem::GetInstance()->Finalize();

    AnimationManager::GetInstance()->Finalize();
    CameraManager::GetInstance()->Finalize();
    lightManager_->Finalize();

#ifdef _DEBUG
    ImGuiManager::GetInstance()->Finalize();
#endif // _DEBUG
    ShaderManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    FontManager::GetInstance()->Finalize();

    // 深度バッファは DSV ヒープへ登録されたディスクリプタを介して参照されているため、
    // ヒープ本体（dsvHeap_->Finalize()）より先にリソースとディスクリプタを解放する
    dsvResource_.Finalize();
    dsvHeap_->ReleaseDescriptor(dxDsv_);

    // スワップチェーン・コマンドはデバイスに依存するオブジェクトなので、デバイス解放より先に片付ける。
    // フェンスは実行中のコマンドの完了を保証する仕組みであるため、コマンドを閉じた直後に解放する。
    dxSwapChain_->Finalize();
    dxCommand_->Finalize();
    DxCommand::ResetAll();
    dxFence_->Finalize();

    // 各種グローバルディスクリプタヒープの解放（ヒープを参照しているリソース側は既に片付いている前提）
    dsvHeap_->Finalize();
    rtvHeap_->Finalize();
    srvHeap_->Finalize();

    // 上記すべてのDX12オブジェクトが依存していたデバイスは最後に解放する
    dxDevice_->Finalize();

    input_->Finalize();
    Audio::StaticFinalize();

    // フレーム間で保持していたグローバルなリソース状態追跡テーブルをクリアし、
    // 次回起動時（あるいは再初期化時）に古い状態情報が残らないようにする
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
        deltaTimer_->SetDeltaTime(Config::Time::kMaxDeltaTime);
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

    MessageBus::GetInstance()->Update(deltaTimer_->GetDeltaTime());

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
