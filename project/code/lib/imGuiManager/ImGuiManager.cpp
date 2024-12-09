#include "ImGuiManager.h"

#include "Engine.h"
#include "winApp/WinApp.h"

#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxHeap.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/DxSwapChain.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#endif // _DEBUG

ImGuiManager* ImGuiManager::getInstance(){
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Init(const WinApp* window,const DxDevice* dxDevice,const DxSwapChain* dxSwapChain){
 #ifdef _DEBUG
	srvHeap_ = DxHeap::getInstance()->getSrvHeap();

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(),"main","main");

	// 先頭のDescriptorを使っている事になっているので合わせる
	// 追記，fontのテクスチャに使われているらしい
	dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

	///=============================================
	/// imgui の初期化
	///=============================================
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window->getHwnd());
	ImGui_ImplDx12_Init(
		dxDevice->getDevice(),
		dxSwapChain->getBufferCount(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeap_,
		srvHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvHeap_->GetGPUDescriptorHandleForHeapStart()
	);
	ImGuiIO& io = ImGui::GetIO();
	// Docking を可能に
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// ctl + Mouse Wheel で フォントサイズを 変更可能に
	io.FontAllowUserScaling = true;

	io.Fonts->AddFontFromFileTTF("resource/fonts/FiraMono-Regular.ttf",16.0f);
#endif // _DEBUG
}

void ImGuiManager::Finalize(){
#ifdef _DEBUG
	dxCommand_->Finalize();
	dxSrvArray_->Finalize();

	ImGui_ImplDx12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // _DEBUG
}

void ImGuiManager::Begin(){
#ifdef _DEBUG
	ImGui_ImplDx12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // _DEBUG
}

void ImGuiManager::End(){
#ifdef _DEBUG
#endif
}

void ImGuiManager::Draw(){
#ifdef _DEBUG
	// 描画前準備
	ImGui::Render();

	ID3D12DescriptorHeap* ppHeaps[] = {srvHeap_};
	dxCommand_->getCommandList()->SetDescriptorHeaps(1,ppHeaps);

	ImGui_ImplDx12_RenderDrawData(ImGui::GetDrawData(),dxCommand_->getCommandList());
#endif // _DEBUG
}