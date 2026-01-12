#include "ImGuiManager.h"

/// engine
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

#include "winApp/WinApp.h"

// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxSwapChain.h"

/// externals
#ifdef _DEBUG
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/ImGuizmo/ImGuizmo.h>

#endif // _DEBUG

using namespace OriGine;

/// <summary> シングルトンインスタンスを取得する. </summary>
ImGuiManager* ImGuiManager::GetInstance() {
    static ImGuiManager instance;
    return &instance;
}

/// <summary>
/// ImGui の初期化.
/// SRV ヒープの取得、コマンド管理の初期化、ImGui コンテキストの作成、Win32/DX12 実装の初期化を行う.
/// </summary>
void ImGuiManager::Initialize([[maybe_unused]] const WinApp* window, [[maybe_unused]] const DxDevice* dxDevice, [[maybe_unused]] const DxSwapChain* dxSwapChain) {
#ifdef _DEBUG
    // エンジンから SRV ヒープを取得
    srvHeap_ = Engine::GetInstance()->GetSrvHeap()->GetHeap();

    // ImGui 用のコマンドバッファ管理を初期化
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // フォントテクスチャ用に記述子を 1 つ割り当てる
    srv_ = Engine::GetInstance()->GetSrvHeap()->AllocateDescriptor();

    ///=============================================
    /// imgui の初期化
    ///=============================================
    IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    ::ImGui::StyleColorsDark();

    // Win32 と DX12 のバックエンドを初期化
    ImGui_ImplWin32_Init(window->GetHwnd());
    ImGui_ImplDx12_Init(
        dxDevice->device_.Get(),
        dxSwapChain->GetBufferCount(),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvHeap_.Get(),
        srv_.GetCpuHandle(),
        srv_.GetGpuHandle());

    ImGuiIO& io = ImGui::GetIO();
    // ドッキング機能を有効化
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Ctrl + マウスホイールでフォントサイズを変更可能にする
    io.FontAllowUserScaling = true;

    // フォントの読み込み
    std::string fontPath = kEngineResourceDirectory + "/fonts/FiraMono-Regular.ttf";
    font_                = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

#endif // _DEBUG
}

/// <summary>
/// ImGui の終了処理. バックエンドのシャットダウンとリソースの解放を行う.
/// </summary>
void ImGuiManager::Finalize() {
#ifdef _DEBUG
    dxCommand_->Finalize();

    ImGui_ImplDx12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 割り当てた記述子を解放
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv_);
    srvHeap_.Reset();
#endif // _DEBUG
}

/// <summary>
/// 新しい ImGui フレームを開始する.
/// </summary>
void ImGuiManager::Begin() {
#ifdef _DEBUG
    ImGui_ImplDx12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif // _DEBUG
}

/// <summary>
/// フレームの終了処理（現在は何も行わない）.
/// </summary>
void ImGuiManager::End() {
#ifdef _DEBUG
#endif
}

/// <summary>
/// 生成された描画データを DX12 コマンドリストに発行する.
/// </summary>
void ImGuiManager::Draw() {
#ifdef _DEBUG
    // 描画データの生成
    ImGui::Render();

    // 記述子ヒープをセット
    ID3D12DescriptorHeap* ppHeaps[] = {srvHeap_.Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);

    // 描画コマンドの発行
    ImGui_ImplDx12_RenderDrawData(ImGui::GetDrawData(), dxCommand_->GetCommandList().Get());
#endif // _DEBUG
}
