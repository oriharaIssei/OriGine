#include "directX12/DxFunctionHelper.h"

/// stl
#include <cassert>

/// lib
#include "logger/Logger.h"

void DxFunctionHelper::SetViewportsAndScissor(const DxCommand* dxCommand, const WinApp* window) {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();
    // ビューポートの設定
    D3D12_VIEWPORT viewPort{};
    viewPort.Width    = static_cast<float>(window->getWidth());
    viewPort.Height   = static_cast<float>(window->getHeight());
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewPort);

    D3D12_RECT scissorRect{};
    scissorRect.left   = 0;
    scissorRect.right  = window->getWidth();
    scissorRect.top    = 0;
    scissorRect.bottom = window->getHeight();

    commandList->RSSetScissorRects(1, &scissorRect);
}

void DxFunctionHelper::SetViewportsAndScissor(const DxCommand* dxCommand, const Vec2f& rectSize) {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();
    // ビューポートの設定
    D3D12_VIEWPORT viewPort{};
    viewPort.Width    = rectSize[X];
    viewPort.Height   = rectSize[Y];
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewPort);

    D3D12_RECT scissorRect{};
    scissorRect.left   = 0;
    scissorRect.right  = static_cast<LONG>(rectSize[X]);
    scissorRect.top    = 0;
    scissorRect.bottom = static_cast<LONG>(rectSize[Y]);

    commandList->RSSetScissorRects(1, &scissorRect);
}

void DxFunctionHelper::SetRenderTargets(const DxCommand* dxCommand, DxDsvDescriptor* dxDsv, const DxSwapChain* dxSwapChain) {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxSwapChain->getCurrentBackBufferRtv();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDsv->getCpuHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void DxFunctionHelper::SetRenderTargets(const DxCommand* dxCommand, DxDsvDescriptor* dxDsv, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDsv->getCpuHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void DxFunctionHelper::PreDraw(DxCommand* dxCommand, const WinApp* window, DxDsvDescriptor* dxDsv, const DxSwapChain* swapChain) {
    ///=========================================
    //	TransitionBarrierの設定
    ///=========================================
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();

    dxCommand->ResourceBarrier(swapChain->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain->getCurrentBackBufferRtv();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDsv->getCpuHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // ビューポートの設定
    D3D12_VIEWPORT viewPort{};
    viewPort.Width    = static_cast<float>(window->getWidth());
    viewPort.Height   = static_cast<float>(window->getHeight());
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewPort);

    D3D12_RECT scissorRect{};
    scissorRect.left   = 0;
    scissorRect.right  = window->getWidth();
    scissorRect.top    = 0;
    scissorRect.bottom = window->getHeight();

    commandList->RSSetScissorRects(1, &scissorRect);

    swapChain->CurrentBackBufferClear(dxCommand, dxDsv);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxFunctionHelper::PreDraw(DxCommand* dxCommand, const Vec2f& rectSize, DxDsvDescriptor* dxDsv, const DxSwapChain* dxSwapChain) {
    ///=========================================
    //	TransitionBarrierの設定
    ///=========================================
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand->getCommandList();

    dxCommand->ResourceBarrier(dxSwapChain->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxSwapChain->getCurrentBackBufferRtv();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDsv->getCpuHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // ビューポートの設定
    D3D12_VIEWPORT viewPort{};
    viewPort.Width    = static_cast<FLOAT>(rectSize[X]);
    viewPort.Height   = static_cast<FLOAT>(rectSize[Y]);
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewPort);

    D3D12_RECT scissorRect{};
    scissorRect.left   = 0;
    scissorRect.right  = static_cast<LONG>(rectSize[X]);
    scissorRect.top    = 0;
    scissorRect.bottom = static_cast<LONG>(rectSize[Y]);

    commandList->RSSetScissorRects(1, &scissorRect);

    dxSwapChain->CurrentBackBufferClear(dxCommand, dxDsv);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxFunctionHelper::PostDraw(DxCommand* dxCommand, DxFence* fence, DxSwapChain* swapChain) {
    HRESULT hr;
    ///===============================================================
    ///	バリアの更新(描画->表示状態)
    ///===============================================================
    dxCommand->ResourceBarrier(
        swapChain->getCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT);
    ///===============================================================

    // コマンドの受付終了 -----------------------------------
    hr = dxCommand->Close();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(hr));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand->ExecuteCommand();
    ///===============================================================

    swapChain->Present();

    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    fence->Signal(dxCommand->getCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand->CommandReset();
    ///===============================================================
}
