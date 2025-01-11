#include "DxResource.h"

#include "directX12/DxDevice.h"

#include <cassert>

void DxResource::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
    //頂点リソース用のヒープの設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; //UploadHeapを使う
    //頂点リソースの設定
    D3D12_RESOURCE_DESC vertexResourceDesc{};
    //バッファのリソース(テクスチャの場合は別の設定をする)
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexResourceDesc.Width     = sizeInBytes;
    //バッファの場合、これらは 1 にする
    vertexResourceDesc.Height           = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels        = 1;
    vertexResourceDesc.SampleDesc.Count = 1;
    //バッファの場合はこれにする
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource_));
    assert(SUCCEEDED(hr));
}

void DxResource::CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vec4f& clearColor) {
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width               = static_cast<UINT64>(width);
    resourceDesc.Height              = static_cast<UINT>(height);
    resourceDesc.DepthOrArraySize    = 1;
    resourceDesc.MipLevels           = 1;
    resourceDesc.Format              = format;
    resourceDesc.SampleDesc.Count    = 1;
    resourceDesc.SampleDesc.Quality  = 0;
    resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags               = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_HEAP_PROPERTIES heapProps{};
    // VRAM 上に 生成
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format   = format;
    clearValue.Color[0] = clearColor.x();
    clearValue.Color[1] = clearColor.y();
    clearValue.Color[2] = clearColor.z();
    clearValue.Color[3] = clearColor.w();

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, // 描画すること を 前提とした テクスチャ なので RenderTarget として 扱う
        &clearValue,
        IID_PPV_ARGS(&resource_));
}

void DxResource::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
    //================================================
    // 1. metadata を基に Resource を設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = UINT(metadata.width);
    resourceDesc.Height           = UINT(metadata.height);
    resourceDesc.MipLevels        = UINT16(metadata.mipLevels); // mipMap の数
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行 or Texture[]の配列数
    resourceDesc.Format           = metadata.format;            //texture の Format
    resourceDesc.SampleDesc.Count = 1;                          // サンプリングカウント 1固定
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION(metadata.dimension);

    //================================================
    // 2. 利用する Heap の設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    //================================================
    // 3. Resource の作成
    HRESULT hr;
    hr = device->CreateCommittedResource(
        &heapProperties, // heap の設定
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, // Clear最適値
        IID_PPV_ARGS(&resource_));
    assert(SUCCEEDED(hr));
}

void DxResource::Finalize() {
    resource_.Reset();
}
