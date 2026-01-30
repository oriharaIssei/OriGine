#include "directX12/DxSwapChain.h"

/// stl
#include <cassert>
#include <format>

/// engine
#include "Engine.h"
#include "EngineConfig.h"
#include "winApp/WinApp.h"
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
/// util
#include "logger/Logger.h"
#include "util/StringUtil.h"

using namespace OriGine;

void DxSwapChain::Initialize(const WinApp* _winApp, const DxDevice* _device, const DxCommand* _command) {
    bufferWidth_  = _winApp->GetWidth();
    bufferHeight_ = _winApp->GetHeight();

    ///================================================
    ///	SwapChain の生成
    ///================================================
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
    swapchainDesc.Width            = bufferWidth_; // 画面の幅。windowと同じにする
    swapchainDesc.Height           = bufferHeight_; // 画面の高さ。windowと同じにする
    swapchainDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.SampleDesc.Count = 1; // マルチサンプルしない
    swapchainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
    swapchainDesc.BufferCount      = Config::Rendering::kSwapChainBufferCount; // ダブルバッファ
    swapchainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら中身を破棄する

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT result = _device->dxgiFactory_->CreateSwapChainForHwnd(
        _command->GetCommandQueue().Get(),
        _winApp->GetHwnd(), // 描画対象のWindowのハンドル
        &swapchainDesc,
        nullptr,
        nullptr,
        &swapChain1);
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to create swap chain. \n Message : {}", HrToString(result));
        assert(false);
    }

    // swapChain1から SwapChain4を得る
    result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain_));
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to query swap chain interface. \n Message : {}", HrToString(result));
        assert(false);
    }

    ///================================================
    ///	Resource の初期化
    ///================================================
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    bufferCount_ = Config::Rendering::kSwapChainBufferCount;
    backBufferResources_.resize(bufferCount_);

    auto* rtvHeap = Engine::GetInstance()->GetRtvHeap();
    for (int i = 0; i < (int)bufferCount_; ++i) {
        result = swapChain_->GetBuffer(
            i, IID_PPV_ARGS(backBufferResources_[i].GetResourceRef().GetAddressOf()));

        if (FAILED(result)) {
            LOG_CRITICAL("Failed to Get swap chain buffer. \n Message : {}", HrToString(result));
            assert(false);
        }

        // バッファに名前を付ける
        std::wstring name = std::format(L"SwapChainBuffer[{}]", i);
        backBufferResources_[i].SetName(name.c_str());

        RTVEntry rtvEntry{
            &backBufferResources_[i],
            rtvDesc};

        backBuffers_.emplace_back(rtvHeap->CreateDescriptor(&rtvEntry));
    }
    ///================================================
}

void DxSwapChain::Finalize() {
    auto* rtvHeap = Engine::GetInstance()->GetRtvHeap();
    for (int i = 0; i < (int)bufferCount_; ++i) {
        if (backBuffers_[i].GetIndex() >= 0) {
            rtvHeap->ReleaseDescriptor(backBuffers_[i]);
        }
        backBufferResources_[i].Finalize();
    }

    swapChain_.Reset();
}

void DxSwapChain::Present() {
    swapChain_->Present(1, 0);
}

void DxSwapChain::CurrentBackBufferClear(DxCommand* _commandList, const DxDsvDescriptor& _dsv) const {
    _commandList->ClearTarget(backBuffers_[swapChain_->GetCurrentBackBufferIndex()], _dsv, clearColor_);
}

void DxSwapChain::ResizeBuffer(UINT _width, UINT _height) {
    if (_width == bufferWidth_ && _height == bufferHeight_) {
        return;
    }
    bufferWidth_  = _width;
    bufferHeight_ = _height;

    // 古いバックバッファを解放
    auto* rtvHeap = Engine::GetInstance()->GetRtvHeap();
    for (int i = 0; i < (int)bufferCount_; ++i) {
        if (backBuffers_[i].GetIndex() >= 0) {
            rtvHeap->ReleaseDescriptor(backBuffers_[i]);
        }
        backBufferResources_[i].Finalize();
    }

    // バッファのリサイズ
    HRESULT result = swapChain_->ResizeBuffers(
        bufferCount_, // バッファの数
        bufferWidth_, // 新しい幅
        bufferHeight_, // 新しい高さ
        DXGI_FORMAT_R8G8B8A8_UNORM, // フォーマット
        0 // フラグ
    );

    if (FAILED(result)) {
        LOG_CRITICAL("Failed to Resize swap chain buffers. \n Message : {}", HrToString(result));
        assert(false);
    }

    // 新しいバックバッファを取得
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (UINT i = 0; i < bufferCount_; ++i) {
        result = swapChain_->GetBuffer(i, IID_PPV_ARGS(backBufferResources_[i].GetResourceRef().GetAddressOf()));

        if (FAILED(result)) {
            LOG_CRITICAL("Failed to Get swap chain buffer after Resize. \n Message : {}", HrToString(result));
            assert(false);
        }

        // バッファに名前を付ける
        RTVEntry rtvEntry{
            &backBufferResources_[i],
            rtvDesc};

        backBuffers_[i]   = rtvHeap->CreateDescriptor(&rtvEntry);
        std::wstring name = std::format(L"SwapChainBuffer[{}]", i);
        backBufferResources_[i].SetName(name.c_str());
    }
}
