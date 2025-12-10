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
/// DirectX12 関数群 ヘルパークラス
/// </summary>
class DxFunctionHelper {
public:
    /// <summary>
    /// ビューポートとシザー矩形をwindowに合わせて設定
    /// </summary>
    /// <param name="dxCommand"></param>
    /// <param name="window"></param>
    static void SetViewportsAndScissor(const DxCommand* dxCommand, const WinApp* window);
    /// <summary>
    /// ビューポートとシザー矩形の設定
    /// </summary>
    static void SetViewportsAndScissor(const DxCommand* dxCommand, const Vec2f& rectSize);

    /// <summary>
    /// レンダーターゲットとデプスステンシルビューの設定
    /// </summary>
    static void SetRenderTargets(const DxCommand* dxCommand, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);
    /// <summary>
    /// レンダーターゲットとデプスステンシルビューの設定
    /// </summary>
    static void SetRenderTargets(const DxCommand* dxCommand, const DxDsvDescriptor& dxDsv, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    /// <summary>
    /// 描画前処理
    /// </summary>
    static void PreDraw(DxCommand* dxCommand, const WinApp* window, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);
    /// <summary>
    /// 描画前処理
    /// </summary>
    static void PreDraw(DxCommand* dxCommand, const Vec2f& rectSize, const DxDsvDescriptor& dxDsv, const DxSwapChain* dxSwapChain);

    /// <summary>
    /// 描画後処理
    /// </summary>
    static void PostDraw(DxCommand* dxCommand, DxFence* dxFence, DxSwapChain* dxSwapChain);
};
using DxFH = DxFunctionHelper;

} // namespace OriGine
