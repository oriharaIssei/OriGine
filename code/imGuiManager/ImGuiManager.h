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
/// ImGui のライフサイクルとリソースを管理するシングルトンクラス.
/// エンジンの初期化・更新・描画の各フェーズで ImGui の処理を呼び出す.
/// </summary>
class ImGuiManager {
public:
    /// <summary> インスタンスの取得. </summary>
    static ImGuiManager* GetInstance();

    /// <summary>
    /// ImGuiContext の作成、Win32/DX12 実装の初期化、フォントのセットアップを行う.
    /// </summary>
    /// <param name="_window">メインウィンドウのインスタンス</param>
    /// <param name="_dxDevice">D3D12 デバイスのインスタンス</param>
    /// <param name="_dxSwapChain">スワップチェーンのインスタンス</param>
    void Initialize(const WinApp* _window, const DxDevice* _dxDevice, const DxSwapChain* _dxSwapChain);

    /// <summary>
    /// ImGuiContext の破棄と DX12 リソースの解放を行う.
    /// </summary>
    void Finalize();

    /// <summary>
    /// ImGui の新しいフレームを開始する.
    /// </summary>
    void Begin();

    /// <summary>
    /// ImGui の描画データ生成などの終了処理を行う.
    /// </summary>
    void End();

    /// <summary>
    /// コマンドリストに ImGui の描画コマンドを積む.
    /// </summary>
    void Draw();

private:
    ImGuiManager()                                     = default;
    ~ImGuiManager()                                    = default;
    ImGuiManager(const ImGuiManager&)                  = delete;
    const ImGuiManager& operator=(const ImGuiManager&) = delete;
#ifdef _DEBUG
private:
    // SRV用ヒープ（ImGui がテクスチャ描画に使用する）
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
    DxSrvDescriptor srv_; // フォントテクスチャ用等の記述子領域
    std::unique_ptr<DxCommand> dxCommand_; // ImGui 描画用のコマンド管理

    // ImGuiのフォントデータ
    ImFont* font_             = nullptr;
    ImFont* materialIconFont_ = nullptr;

public:
    /// <summary> SRV ヒープの取得. </summary>
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSrvHeap() const { return srvHeap_; }
    /// <summary> 内部 SRV 記述子の取得. </summary>
    const DxSrvDescriptor& GetSrv() const { return srv_; }
    /// <summary> ImGui 用 DX12 コマンド管理オブジェクトの取得. </summary>
    DxCommand* GetDxCommand() { return dxCommand_.get(); }

    /// <summary> 標準フォントの取得. </summary>
    ImFont* GetFont() const { return font_; }
    /// <summary> マテリアルアイコンフォントの取得. </summary>
    ImFont* GetMaterialIconFont() const { return materialIconFont_; }

    /// <summary> 指定したフォントをスタックに積む. </summary>
    void pushFont(ImFont* _font) {
        ImGui::PushFont(_font);
    }
    /// <summary> 標準フォントをスタックに積む. </summary>
    void pushFont() {
        ImGui::PushFont(font_);
    }
    /// <summary> マテリアルアイコンフォントをスタックに積む. </summary>
    void pushFontMaterialIcon() {
        ImGui::PushFont(materialIconFont_);
    }
#endif // _DEBUG
};

} // namespace OriGine
