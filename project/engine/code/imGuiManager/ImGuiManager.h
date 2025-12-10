#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <memory>

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"

/// external
#ifdef _DEBUG
struct ImFont;
#endif // _DEBUG

namespace OriGine {
/// 前方宣言

/// engine
class WinApp;
// directX12
struct DxDevice;
class DxSwapChain;
class DxCommand;

/// <summary>
/// ImGui の管理クラス
/// </summary>
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

} // namespace OriGine
