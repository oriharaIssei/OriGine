#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXTex/DirectXTex.h"
class DxDevice;

#include "Vector4.h"
#include <stdint.h>

#include <util/BitArray.h>
#include <util/EnumBitMask.h>

enum class DxResourceType : int32_t {
    Unknown            = 0b0,
    Descriptor_RTV     = 0b1 << 1,
    Descriptor_DSV     = 0b1 << 2,
    Descriptor_SRV     = 0b1 << 3,
    Descriptor_UAV     = 0b1 << 4,
    Descriptor_Sampler = 0b1 << 5,
    Buffer             = 0b1 << 6,
    Buffer_Constant    = 0b1 << 8,
    Buffer_Structured  = 0b1 << 9
};

/// <summary>
/// DxResourceType を 文字列に変換
/// </summary>
const char* DxResourceTypeToString(DxResourceType type);
namespace std {
string to_string(DxResourceType type);
};

/// <summary>
/// DirectX12 リソースの WrapperClass
/// </summary>
class DxResource {
public:
    DxResource()  = default;
    ~DxResource() = default;

    /// <summary>
    /// BufferResource として 作成
    /// </summary>
    /// <param name="device"></param>
    /// <param name="sizeInBytes">bufferのサイズ</param>
    void CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

    /// <summary>
    /// DSVBufferResource として 作成
    /// </summary>
    /// <param name="_device"></param>
    /// <param name="_width"></param>
    /// <param name="_height"></param>
    void CreateDSVBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, UINT64 _width, UINT _height);

    /// <summary>
    /// UAVBufferResource として 作成
    /// </summary>
    void CreateUAVBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);
    /// <summary>
    /// RenderTextureResource として 作成
    /// </summary>
    void CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vec4f& clearColor);
    /// <summary>
    /// TextureResource として 作成
    /// </summary>
    /// <param name="device"></param>
    /// <param name="metadata"></param>
    void CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    EnumBitmask<DxResourceType> type_                = DxResourceType::Unknown; // リソースの種類
    D3D12_RESOURCE_DESC resourceDesc_{}; // リソースの詳細情報

public:
    const EnumBitmask<DxResourceType>& GetType() const { return type_; }
    void AddType(DxResourceType _type) { type_ |= EnumBitmask(_type); }
    void SetType(DxResourceType _type) { type_ = EnumBitmask(_type); }

    /// <summary>
    /// Resourceが有効かどうかを確認
    /// </summary>
    /// <returns></returns>
    bool IsValid() const { return resource_ != nullptr; }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource() const { return resource_; }
    Microsoft::WRL::ComPtr<ID3D12Resource>& GetResourceRef() { return resource_; }

    const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc_; }
    UINT64 GetSizeInBytes() const { return resourceDesc_.Width; } // バッファのサイズを取得
    UINT64 GetWidth() const { return resourceDesc_.Width; } // テクスチャの幅を取得
    UINT GetHeight() const { return resourceDesc_.Height; } // テクスチャの高さを取得

    HRESULT SetName(const std::wstring& name);
};
