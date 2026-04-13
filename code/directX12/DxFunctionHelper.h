#pragma once

/// Microsoft
#include <wrl.h>

/// engine
#include "winApp/WinApp.h"
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/DxSwapChain.h"
/// math
#include "Vector2.h"
#include "Vector4.h"

namespace OriGine {

/// <summary>
/// DirectX 12 に関する定型的な一連の命令 (ビューポート設定、レンダーターゲットのバインド、描画開始・終了処理など) を
/// 簡略化するためのスタティックなヘルパークラス. 別名 `DxFH` として利用可能.
/// </summary>
class DxFunctionHelper {
public:
    /// <summary>
    /// ウィンドウのクライアント領域に合わせてビューポートとシザー矩形を設定する.
    /// </summary>
    /// <param name="dxCommand">コマンドリストを保持するオブジェクト</param>
    /// <param name="window">対象のウィンドウ</param>
    static void SetViewportsAndScissor(const DxCommand* dxCommand, const WinApp* window);

    /// <summary>
    /// 指定されたサイズでビューポートとシザー矩形を設定する.
    /// </summary>
    /// <param name="dxCommand">コマンドリスト</param>
    /// <param name="rectSize">ビューポートのサイズ (幅, 高さ)</param>
    static void SetViewportsAndScissor(const DxCommand* dxCommand, const Vec2f& rectSize);

    /// <summary>
    /// 指定された RTV (スワップチェインのバックバッファ) と DSV をレンダーターゲットとして設定する.
    /// </summary>
    /// <param name="dxCommand">コマンドリスト</param>
    /// <param name="dxDsv">デプスステンシルビューのディスクリプタ</param>
    /// <param name="dxSwapChain">スワップチェイン (現在のバックバッファを参照する)</param>
    static void SetRenderTargets(const DxCommand* dxCommand, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);

    /// <summary>
    /// 任意の RTV ハンドルと DSV をレンダーターゲットとして設定する. (オフスクリーンレンダリング用)
    /// </summary>
    /// <param name="dxCommand">コマンドリスト</param>
    /// <param name="dxDsv">DSV ディスクリプタ</param>
    /// <param name="rtvHandle">RTV の CPU ディスクリプタハンドル</param>
    static void SetRenderTargets(const DxCommand* dxCommand, const DxDsvDescriptor& dxDsv, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    /// <summary>
    /// 描画開始前の準備処理を行う (コマンドアロケータのリセット、コマンドリストのリセット、バックバッファのクリア、DSV のクリアなど).
    /// </summary>
    /// <param name="dxCommand">コマンドリスト</param>
    /// <param name="window">ウィンドウ</param>
    /// <param name="dxDsv">使用する DSV</param>
    /// <param name="dxSwapChain">使用するスワップチェイン</param>
    static void PreDraw(DxCommand* dxCommand, const WinApp* window, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);

    /// <summary>
    /// 描画開始前の準備処理を行う (サイズ指定版).
    /// </summary>
    static void PreDraw(DxCommand* dxCommand, const Vec2f& rectSize, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);

    /// <summary>
    /// 描画終了後の後処理を行う (リソースバリアの Present 遷移、コマンドリストのクローズ、実行、スワップチェインの表示、フェンス待機など).
    /// </summary>
    /// <param name="dxCommand">コマンドリスト</param>
    /// <param name="dxFence">同期待ちに使用するフェンス</param>
    /// <param name="dxSwapChain">画面送りを行うスワップチェイン</param>
    static void PostDraw(DxCommand* dxCommand, DxFence* dxFence, DxSwapChain* dxSwapChain);
};
/// <summary>DxFunctionHelper の略称エイリアス</summary>
using DxFH = DxFunctionHelper;

} // namespace OriGine
