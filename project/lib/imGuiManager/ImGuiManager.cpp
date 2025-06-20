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

ImGuiManager* ImGuiManager::getInstance() {
    static ImGuiManager instance;
    return &instance;
}

void ImGuiManager::Initialize([[maybe_unused]] const WinApp* window, [[maybe_unused]] const DxDevice* dxDevice, [[maybe_unused]] const DxSwapChain* dxSwapChain) {
#ifdef _DEBUG
    srvHeap_ = Engine::getInstance()->getSrvHeap()->getHeap();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // 先頭のDescriptorを使っている事になっているので合わせる
    // 追記，fontのテクスチャに使われているらしい

    srv_ = Engine::getInstance()->getSrvHeap()->AllocateDescriptor();

    ///=============================================
    /// imgui の初期化
    ///=============================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(window->getHwnd());
    ImGui_ImplDx12_Init(
        dxDevice->getDevice().Get(),
        dxSwapChain->getBufferCount(),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvHeap_.Get(),
        srv_->getCpuHandle(),
        srv_->getGpuHandle());
    ImGuiIO& io = ImGui::GetIO();
    // Docking を可能に
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // ctl + Mouse Wheel で フォントサイズを 変更可能に
    io.FontAllowUserScaling = true;

    std::string fontPath = kEngineResourceDirectory + "/fonts/FiraMono-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

#endif // _DEBUG
}

void ImGuiManager::Finalize() {
#ifdef _DEBUG
    dxCommand_->Finalize();

    ImGui_ImplDx12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Engine::getInstance()->getSrvHeap()->ReleaseDescriptor(srv_);
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
    dxCommand_->getCommandList()->SetDescriptorHeaps(1, ppHeaps);

    ImGui_ImplDx12_RenderDrawData(ImGui::GetDrawData(), dxCommand_->getCommandList().Get());
#endif // _DEBUG
}
