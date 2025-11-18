#include "DxResource.h"

/// stl
#include <cassert>

/// engine
// directX12 object
#include "directX12/DxDevice.h"

///externals
#include "logger/Logger.h"

const char* DxResourceTypeToString(DxResourceType type) {
    switch (type) {
    case DxResourceType::Unknown:
        return "Unknown";
    case DxResourceType::Descriptor_RTV:
        return "Descriptor_RTV";
    case DxResourceType::Descriptor_DSV:
        return "Descriptor_DSV";
    case DxResourceType::Descriptor_SRV:
        return "Descriptor_SRV";
    case DxResourceType::Descriptor_UAV:
        return "Descriptor_UAV";
    case DxResourceType::Descriptor_Sampler:
        return "Descriptor_Sampler";
    case DxResourceType::Buffer:
        return "Buffer";
    case DxResourceType::Buffer_Constant:
        return "Buffer_Constant";
    case DxResourceType::Buffer_Structured:
        return "Buffer_Structured";
    default:
        return "Unknown Type";
    }
}

std::string std::to_string(DxResourceType type) {
    return DxResourceTypeToString(type);
}

void DxResource::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {
    // リソース用のヒープの設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
    // リソースの設定
    // バッファのリソース(テクスチャの場合は別の設定をする)
    resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc_.Width     = sizeInBytes;
    // バッファの場合、これらは 1 にする
    resourceDesc_.Height           = 1;
    resourceDesc_.DepthOrArraySize = 1;
    resourceDesc_.MipLevels        = 1;
    resourceDesc_.SampleDesc.Count = 1;
    // バッファの場合はこれにする
    resourceDesc_.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc_,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource_));

    if (FAILED(hr)) {
        LOG_CRITICAL("Failed to create buffer resource.\n massage : {}", std::to_string(hr));
        assert(false);
    }
}

void DxResource::CreateUAVBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags, D3D12_HEAP_TYPE heapType) {
    // Heap Properties の設定
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = heapType; // VRAM 上に生成

    // Resource Desc の設定
    resourceDesc_.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファリソース
    resourceDesc_.Width            = sizeInBytes; // サイズを指定
    resourceDesc_.Height           = 1; // バッファなので高さは 1
    resourceDesc_.DepthOrArraySize = 1; // バッファなので奥行きは 1
    resourceDesc_.MipLevels        = 1; // Mipレベルは 1
    resourceDesc_.SampleDesc.Count = 1; // サンプル数は 1
    resourceDesc_.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // バッファなので行メジャー
    resourceDesc_.Flags            = flags; // UAV 用のフラグを指定

    // Resource の作成
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc_,
        D3D12_RESOURCE_STATE_COMMON, // UAV 用の状態
        nullptr, // Clear最適値は nullptr
        IID_PPV_ARGS(resource_.GetAddressOf()));

    if (FAILED(hr)) {
        LOG_CRITICAL("Failed to create UAV buffer resource.\n massage : {}", std::to_string(hr));
        assert(false);
    }
    type_ |= DxResourceType::Descriptor_UAV;
}

void DxResource::CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vec4f& clearColor) {
    resourceDesc_.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc_.Width              = static_cast<UINT64>(width);
    resourceDesc_.Height             = static_cast<UINT>(height);
    resourceDesc_.DepthOrArraySize   = 1;
    resourceDesc_.MipLevels          = 1;
    resourceDesc_.Format             = format;
    resourceDesc_.SampleDesc.Count   = 1;
    resourceDesc_.SampleDesc.Quality = 0;
    resourceDesc_.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc_.Flags              = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_HEAP_PROPERTIES heapProps{};
    // VRAM 上に 生成
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format   = format;
    clearValue.Color[0] = clearColor[X];
    clearValue.Color[1] = clearColor[Y];
    clearValue.Color[2] = clearColor[Z];
    clearValue.Color[3] = clearColor[W];

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc_,
        D3D12_RESOURCE_STATE_RENDER_TARGET, // 描画すること を 前提とした テクスチャ なので RenderTarget として 扱う
        &clearValue,
        IID_PPV_ARGS(&resource_));

    if (FAILED(hr)) {
        LOG_CRITICAL("Failed to create render texture resource.\n massage :{}", std::to_string(hr));
        HRESULT reason = device->GetDeviceRemovedReason();
        char buf[256];
        sprintf_s(buf, "CreateRenderTextureResource FAILED hr=0x%08X, DeviceRemovedReason=0x%08X\n",
            static_cast<unsigned int>(hr), static_cast<unsigned int>(reason));
        OutputDebugStringA(buf);

        assert(false);
    }
}

void DxResource::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata) {
    //================================================
    // 1. metadata を基に Resource を設定

    resourceDesc_.Width            = UINT(metadata.width);
    resourceDesc_.Height           = UINT(metadata.height);
    resourceDesc_.MipLevels        = UINT16(metadata.mipLevels); // mipMap の数
    resourceDesc_.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行 or Texture[]の配列数
    resourceDesc_.Format           = metadata.format; // texture の Format
    resourceDesc_.SampleDesc.Count = 1; // サンプリングカウント 1固定
    resourceDesc_.Dimension        = D3D12_RESOURCE_DIMENSION(metadata.dimension);

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
        &resourceDesc_,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, // Clear最適値
        IID_PPV_ARGS(&resource_));

    if (FAILED(hr)) {
        LOG_CRITICAL("Failed to create texture resource.\n massage : {}", std::to_string(hr));
        assert(false);
    }
}

void DxResource::Finalize() {
    resource_.Reset();
    type_         = DxResourceType::Unknown; // リソースの種類をリセット
    resourceDesc_ = {}; // リソースの詳細情報をリセット
}

HRESULT DxResource::SetName(const std::wstring& name) {
    HRESULT result = 0;
    if (resource_) {
        result = resource_->SetName(name.c_str());
    }
    if (FAILED(result)) {
        LOG_CRITICAL("Failed to Set resource name.\n massage : {}", result);
        assert(false);
    }
    return result;
}
