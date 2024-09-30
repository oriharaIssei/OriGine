#pragma once

#include <memory>
#include <wrl.h>

#include <d3d12.h>

#include "directX12/dxResource/srv/DxSrvArray.h"

class WinApp;
class DxDevice;
class DxSwapChain;
class DxCommand;

class ImGuiManager{
public:
	static ImGuiManager *getInstance();

	void Init(const WinApp *window,const DxDevice *dxDevice,const DxSwapChain *dxSwapChain);
	void Finalize();

	void Begin();
	void End();

	void Draw();
private:
#ifdef _DEBUG
	// SRV用ヒープ
	ID3D12DescriptorHeap *srvHeap_;

	std::shared_ptr<DxSrvArray>dxSrvArray_;

	std::unique_ptr<DxCommand> dxCommand_;
#endif // _DEBUG

	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager &) = delete;
	const ImGuiManager &operator=(const ImGuiManager &) = delete;
};