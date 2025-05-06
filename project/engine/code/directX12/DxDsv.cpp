#include "directX12/DxDsv.h"

/// stl
#include <assert.h>

/// lib
#include <logger/Logger.h>

void DxDepthStencilView::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* dsvHeap, UINT64 width, UINT height) {
    width_  = width;
    height_ = height;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = width_;
    resourceDesc.Height           = height_;
    resourceDesc.MipLevels        = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // heap の設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 最大値でクリア
    depthClearValue.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT; // Resource と合わせる

    HRESULT result = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&dsv_));

    if (FAILED(result)) {
        // エラーログを出力
        LOG_ERROR("Failed to create depth stencil view resource.");
        assert(false);
    }

    // DSV の設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT; // resourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2d Texture

    // DsvHeap の先頭に DSV を作る
    device->CreateDepthStencilView(
        dsv_.Get(),
        &dsvDesc,
        dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
void DxDepthStencilView::Resize(ID3D12Device* device, ID3D12DescriptorHeap* dsvHeap, UINT64 newWidth, UINT newHeight) {
    if (width_ == newWidth && height_ == newHeight) {
        return; // サイズが変わらない場合は何もしない
    }

    width_  = newWidth;
    height_ = newHeight;

    // 古いリソースを解放
    dsv_.Reset();

    // 新しいリソースの作成
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = newWidth;
    resourceDesc.Height           = newHeight;
    resourceDesc.MipLevels        = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 最大値でクリア
    depthClearValue.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;

    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&dsv_));

    if (FAILED(hr)) {
        // エラーログを出力
        LOG_ERROR("Failed to create depth stencil view resource.");
        assert(false);
    }

    // 新しい DSV の作成
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    device->CreateDepthStencilView(
        dsv_.Get(),
        &dsvDesc,
        dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DxDepthStencilView::Finalize() {
    dsv_.Reset();
}
