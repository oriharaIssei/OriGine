#include "directX12/DxSwapChain.h"

/// stl
#include <cassert>

/// engine
#include "winApp/WinApp.h"
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxRtvArrayManager.h"

/// lib
#include "logger/Logger.h"

void DxSwapChain::Initialize(const WinApp* winApp, const DxDevice* device, const DxCommand* command) {
    bufferWidth_  = winApp->getWidth();
    bufferHeight_ = winApp->getHeight();

    ///================================================
    ///	SwapChain の生成
    ///================================================
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
    swapchainDesc.Width            = bufferWidth_; // 画面の幅。windowと同じにする
    swapchainDesc.Height           = bufferHeight_; // 画面の高さ。windowと同じにする
    swapchainDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.SampleDesc.Count = 1; // マルチサンプルしない
    swapchainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
    swapchainDesc.BufferCount      = 2; // ダブルバッファ
    swapchainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら中身を破棄する

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT result = device->getDxgiFactory()->CreateSwapChainForHwnd(
        command->getCommandQueue(),
        winApp->getHwnd(), // 描画対象のWindowのハンドル
        &swapchainDesc,
        nullptr,
        nullptr,
        &swapChain1);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create swap chain.");
        assert(false);
    }

    // SwapChain4を得る
    result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain_));
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to query swap chain interface.");
        assert(false);
    }

    ///================================================
    ///	Resource の初期化
    ///================================================
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    bufferCount_ = 2;

    backBuffers_ = DxRtvArrayManager::getInstance()->Create(bufferCount_);

    for (int i = 0; i < (int)bufferCount_; ++i) {
        Microsoft::WRL::ComPtr<ID3D12Resource> buff;
        result = swapChain_->GetBuffer(
            i, IID_PPV_ARGS(&buff));

        if (FAILED(result)) {
            LOG_CRITICAL("Failed to get swap chain buffer.");
            assert(false);
        }

        // バッファに名前を付ける
        std::wstring name = std::format(L"SwapChainBuffer[{}]", i);
        buff->SetName(name.c_str());
        backBuffers_->CreateView(device->getDevice(), rtvDesc, buff);
    }
    ///================================================
}

void DxSwapChain::Finalize() {
    backBuffers_->Finalize();
    swapChain_.Reset();
}

void DxSwapChain::Present() {
    swapChain_->Present(1, 0);
}

void DxSwapChain::CurrentBackBufferClear(ID3D12GraphicsCommandList* commandList) const {
    backBuffers_->ClearTarget(swapChain_->GetCurrentBackBufferIndex(), commandList, clearColor_.v);
}

void DxSwapChain::ResizeBuffer(const DxDevice* device, UINT width, UINT height) {
    if (width == bufferWidth_ && height == bufferHeight_) {
        return;
    }
    bufferWidth_  = width;
    bufferHeight_ = height;

    // 古いバックバッファを解放
    backBuffers_->Finalize();

    // バッファのリサイズ
    HRESULT result = swapChain_->ResizeBuffers(
        bufferCount_, // バッファの数
        bufferWidth_, // 新しい幅
        bufferHeight_, // 新しい高さ
        DXGI_FORMAT_R8G8B8A8_UNORM, // フォーマット
        0 // フラグ
    );

    if (FAILED(result)) {
        LOG_CRITICAL("Failed to resize swap chain buffers.");
        assert(false);
    }

    // 新しいバックバッファを取得
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (UINT i = 0; i < bufferCount_; ++i) {
        Microsoft::WRL::ComPtr<ID3D12Resource> buff;
        result = swapChain_->GetBuffer(i, IID_PPV_ARGS(&buff));

        if (FAILED(result)) {
            LOG_CRITICAL("Failed to get swap chain buffer after resize.");
            assert(false);
        }

        // バッファに名前を付ける
        std::wstring name = std::format(L"SwapChainBuffer[{}]", i);
        buff->SetName(name.c_str());
        backBuffers_->CreateView(device->getDevice(), rtvDesc, buff);
    }
}
