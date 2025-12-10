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

ImGuiManager* ImGuiManager::GetInstance() {
    static ImGuiManager instance;
    return &instance;
}

void ImGuiManager::Initialize([[maybe_unused]] const WinApp* window, [[maybe_unused]] const DxDevice* dxDevice, [[maybe_unused]] const DxSwapChain* dxSwapChain) {
#ifdef _DEBUG
    srvHeap_ = Engine::GetInstance()->GetSrvHeap()->GetHeap();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // 先頭のDescriptorを使っている事になっているので合わせる
    // 追記，fontのテクスチャに使われているらしい

    srv_ = Engine::GetInstance()->GetSrvHeap()->AllocateDescriptor();

    ///=============================================
    /// imgui の初期化
    ///=============================================
    IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    ::ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(window->GetHwnd());
    ImGui_ImplDx12_Init(
        dxDevice->device_.Get(),
        dxSwapChain->GetBufferCount(),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvHeap_.Get(),
        srv_.GetCpuHandle(),
        srv_.GetGpuHandle());
    ImGuiIO& io = ImGui::GetIO();
    // Docking を可能に
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // ctl + Mouse Wheel で フォントサイズを 変更可能に
    io.FontAllowUserScaling = true;

    std::string fontPath = kEngineResourceDirectory + "/fonts/FiraMono-Regular.ttf";
    font_                = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

    std::string iconFontPath = kEngineResourceDirectory + "/fonts/GoogleMaterial/Material_Icons_Round/MaterialIconsRound-Regular.otf";
    materialIconFont_        = io.Fonts->AddFontFromFileTTF(
        iconFontPath.c_str(), // フォントファイルパス
        16.0f // フォントサイズ
    );

#endif // _DEBUG
}

void ImGuiManager::Finalize() {
#ifdef _DEBUG
    dxCommand_->Finalize();

    ImGui_ImplDx12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv_);
    srvHeap_.Reset();
#endif // _DEBUG
}

void ImGuiManager::Begin() {
#ifdef _DEBUG
    ImGui_ImplDx12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif // _DEBUG
}

void ImGuiManager::End() {
#ifdef _DEBUG
#endif
}

void ImGuiManager::Draw() {
#ifdef _DEBUG
    // 描画前準備
    ImGui::Render();

    ID3D12DescriptorHeap* ppHeaps[] = {srvHeap_.Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);

    ImGui_ImplDx12_RenderDrawData(ImGui::GetDrawData(), dxCommand_->GetCommandList().Get());
#endif // _DEBUG
}
