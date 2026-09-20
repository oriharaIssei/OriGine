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
#ifdef ORIGINE_EDITOR_ENABLED
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
    /// ImGui を実際に動かすかどうかを設定する（実行時フラグ）.
    /// コンパイル時マクロ(ORIGINE_EDITOR_ENABLED)ではなくここで切り替える理由は2つ:
    /// (1) Phase 4D で Game/Editor が同じ OriGine.dll を共有する計画のため、
    ///     いずれコンパイル時に処理を分けること自体ができなくなる。
    /// (2) Game.exe のベンチマークに ImGui の毎フレームコストを混ぜないため
    ///     （4B で ORIGINE_EDITOR_ENABLED を Debug/Develop 双方に広げた結果、
    ///     ガード無しで呼ばれていた Initialize/Begin/End/Draw が実際に動き出し、
    ///     ベンチが対照群でなくなっていた）。
    /// 呼び出しは ECS_TestGame::Initialize() / ECS_TestEditor::Initialize() の
    /// 先頭（Engine::Initialize() より前）に1箇所ずつだけ置く想定で、
    /// それ以外の場所から書き換えないこと。
    /// </summary>
    /// <param name="_enabled">true なら ImGui を初期化・実行する</param>
    void SetEnabled(bool _enabled) { enabled_ = _enabled; }

    /// <summary> ImGui が実行時に有効化されているかどうか. </summary>
    bool IsEnabled() const { return enabled_; }

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

    // ORIGINE_EDITOR_ENABLED が未定義のビルド(Release)では Initialize/Begin/End/Draw の
    // 中身自体が #ifdef で消えるため、この値を読むコードが存在しなくなる。
    // それでもメンバ自体は常にコンパイルする ―― SetEnabled() を呼ぶ側
    // (ECS_TestGame.cpp / ECS_TestEditor.cpp)がマクロの有無を意識せずに済むようにするため。
    bool enabled_ = false;

#ifdef ORIGINE_EDITOR_ENABLED
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
