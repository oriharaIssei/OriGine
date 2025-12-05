#pragma once

/// microsoft
#include <wrl.h>
// directX12
#include <memory>

/// stl
#include <d3d12.h>

/// engine
class WinApp;
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
class DxDevice;
class DxSwapChain;

/// externals
#include <imgui/imgui.h>

#ifdef _DEBUG
struct ImFont;
#endif // _DEBUG

namespace OriGine {

class ImGuiManager {
public:
    static ImGuiManager* GetInstance();

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
    DxSrvDescriptor srv_;
    std::unique_ptr<DxCommand> dxCommand_;

    // ImGuiのフォント
    ImFont* font_             = nullptr;
    ImFont* materialIconFont_ = nullptr;

public:
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSrvHeap() const { return srvHeap_; }
    const DxSrvDescriptor& GetSrv() const { return srv_; }
    DxCommand* GetDxCommand() { return dxCommand_.get(); }

    ImFont* GetFont() const { return font_; }
    ImFont* GetMaterialIconFont() const { return materialIconFont_; }

    void pushFont(ImFont* font) {
        ImGui::PushFont(font);
    }
    void pushFont() {
        ImGui::PushFont(font_);
    }
    void pushFontMaterialIcon() {
        ImGui::PushFont(materialIconFont_);
    }
#endif // _DEBUG
};

}
