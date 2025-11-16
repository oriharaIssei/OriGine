#pragma once

#include <memory>
#include <wrl.h>

#include <d3d12.h>

#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"

class WinApp;
struct DxDevice;
class DxSwapChain;
class DxCommand;

#ifdef _DEBUG
struct ImFont;
#endif // _DEBUG

class ImGuiManager{
public:
    static ImGuiManager* getInstance();

    void Initialize(const WinApp* window,const DxDevice* dxDevice,const DxSwapChain* dxSwapChain);
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
    ImFont* font_ = nullptr;
    ImFont* materialIconFont_ = nullptr;
public:
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& getSrvHeap() const{ return srvHeap_; }
    const DxSrvDescriptor& getSrv() const{ return srv_; }
    DxCommand* getDxCommand(){ return dxCommand_.get(); }

    ImFont* getFont() const{ return font_; }
    ImFont* getMaterialIconFont() const{ return materialIconFont_; }

    void pushFont(ImFont* font){
        ImGui::PushFont(font);
    }
    void pushFont(){
        ImGui::PushFont(font_);
    }
    void pushFontMaterialIcon(){
        ImGui::PushFont(materialIconFont_);
    }
#endif // _DEBUG
};
