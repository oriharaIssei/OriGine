#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// math
#include "Vector4.h"
#include <stdint.h>

/// util
#include <util/BitArray.h>
#include <util/EnumBitMask.h>

/// externals
#include "DirectXTex/DirectXTex.h"

namespace OriGine {
// 前方宣言
struct DxDevice;

/// <summary>
/// DirectX 12 リソースの種類を識別するためのビットフラグ.
/// リソースがどのディスクリプタ（RTV, DSV, SRV, UAV）に紐付いているか、またはバッファの用途を保持する.
/// </summary>
enum class DxResourceType : int32_t {
    /// <summary>未定義</summary>
    Unknown = 0b0,
    /// <summary>レンダーターゲットビューとして使用中</summary>
    Descriptor_RTV = 0b1 << 1,
    /// <summary>深度ステンシルビューとして使用中</summary>
    Descriptor_DSV = 0b1 << 2,
    /// <summary>シェーダーリソースビューとして使用中</summary>
    Descriptor_SRV = 0b1 << 3,
    /// <summary>順序未指定アクセスビューとして使用中</summary>
    Descriptor_UAV = 0b1 << 4,
    /// <summary>サンプラーとして使用中</summary>
    Descriptor_Sampler = 0b1 << 5,
    /// <summary>一般的なバッファリソース</summary>
    Buffer = 0b1 << 6,
    /// <summary>定数バッファ (CBV)</summary>
    Buffer_Constant = 0b1 << 8,
    /// <summary>構造化バッファ (StructuredBuffer)</summary>
    Buffer_Structured = 0b1 << 9,

};

/// <summary>
/// DxResourceType のフラグをデバッグ用の文字列に変換する.
/// </summary>
/// <param name="type">リソースタイプ</param>
/// <returns>タイプ名文字列</returns>
const char* DxResourceTypeToString(DxResourceType type);

/// <summary>
/// ID3D12Resource をラップし、生成、設定、情報の取得を簡略化するクラス.
/// </summary>
class DxResource {
public:
    DxResource()  = default;
    ~DxResource() = default;

    /// <summary>
    /// 定数バッファや頂点バッファなどの汎用バッファリソースを作成する.
    /// </summary>
    /// <param name="_device">D3D12デバイス</param>
    /// <param name="_sizeInBytes">バッファのサイズ（バイト）</param>
    /// <param name="_state">初期のリソース状態</param>
    /// <param name="_heapType">ヒープの種類 (UPLOAD, DEFAULT, READBACK)</param>
    void CreateBufferResource(
        Microsoft::WRL::ComPtr<ID3D12Device> _device,
        size_t _sizeInBytes,
        D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_HEAP_TYPE _heapType    = D3D12_HEAP_TYPE_UPLOAD);

    /// <summary>
    /// 深度ステンシルバッファ (DSV) として使用するテクスチャリソースを作成する.
    /// </summary>
    /// <param name="_device">D3D12デバイス</param>
    /// <param name="_width">テクスチャの幅</param>
    /// <param name="_height">テクスチャの高さ</param>
    void CreateDSVBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, UINT64 _width, UINT _height);

    /// <summary>
    /// 順序未指定アクセス (UAV) が可能なバッファリソースを作成する.
    /// </summary>
    void CreateUAVBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

    /// <summary>
    /// オフスクリーンレンダリング用のレンダーターゲットテクスチャリソースを作成する.
    /// </summary>
    void CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vec4f& clearColor);

    /// <summary>
    /// 画像ファイル（DirectXTex）から作成されるテクスチャリソースを作成する.
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="metadata">画像メタデータ</param>
    void CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

    /// <summary>
    /// 管理している ID3D12Resource を解放する.
    /// </summary>
    void Finalize();

private:
    /// <summary>D3D12リソース本体</summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    /// <summary>リソースの用途フラグ</summary>
    EnumBitmask<DxResourceType> type_ = DxResourceType::Unknown;
    /// <summary>リソースの構成情報キャッシュ</summary>
    D3D12_RESOURCE_DESC resourceDesc_{};

public:
    /// <summary>リソースの種類（用途）を取得する.</summary>
    const EnumBitmask<DxResourceType>& GetType() const { return type_; }
    /// <summary>リソースの種類（用途）を追加する.</summary>
    void AddType(DxResourceType _type) { type_ |= EnumBitmask(_type); }
    /// <summary>リソースの種類（用途）を直接設定する.</summary>
    void SetType(DxResourceType _type) { type_ = EnumBitmask(_type); }

    /// <summary>リソースが正しく生成されているかを確認する.</summary>
    bool IsValid() const { return resource_ != nullptr; }
    /// <summary>ID3D12Resource オブジェクトを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource() const { return resource_; }
    /// <summary>ID3D12Resource オブジェクトの参照を取得する.</summary>
    Microsoft::WRL::ComPtr<ID3D12Resource>& GetResourceRef() { return resource_; }

    /// <summary>リソースの構成情報を取得する.</summary>
    const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc_; }
    /// <summary>バッファリソースの場合、その全容量（バイト数）を取得する.</summary>
    UINT64 GetSizeInBytes() const { return resourceDesc_.Width; }
    /// <summary>テクスチャリソースの幅を取得する.</summary>
    UINT64 GetWidth() const { return resourceDesc_.Width; }
    /// <summary>テクスチャリソースの高さを取得する.</summary>
    UINT GetHeight() const { return resourceDesc_.Height; }

    /// <summary>
    /// デバッグ用にリソースに名称を設定する.
    /// </summary>
    /// <param name="name">リソース名</param>
    /// <returns>HRESULT</returns>
    HRESULT SetName(const std::wstring& name);
};

} // namespace OriGine

namespace std {
string to_string(OriGine::DxResourceType type);
};
