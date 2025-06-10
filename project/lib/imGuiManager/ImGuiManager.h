#pragma once

#include <memory>
#include <wrl.h>

#include <d3d12.h>

#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"

class WinApp;
class DxDevice;
class DxSwapChain;
class DxCommand;

class ImGuiManager {
public:
    static ImGuiManager* getInstance();

    void Initialize(const WinApp* window, const DxDevice* dxDevice, const DxSwapChain* dxSwapChain);
    void Finalize();

    void Begin();
    void End();

    void Draw();

private:
    ImGuiManager()                                     = default;
    ~ImGuiManager()                                    = default;
    ImGuiManager(const ImGuiManager&)                  = delete;
    const ImGuiManager& operator=(const ImGuiManager&) = delete;
#ifdef _DEBUG
private:
    // SRV用ヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
    std::shared_ptr<DxSrvDescriptor> srv_;
    std::unique_ptr<DxCommand> dxCommand_;
#endif // _DEBUG
};
