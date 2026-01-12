#pragma once

/// stl
#include <cstdint>
#include <d3d12.h>
#include <memory>

/// engine
#include "Engine.h"
// directX12
#include "ConceptHasBuffer.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

namespace OriGine {

/// <summary>
/// GPU上の構造化バッファ (StructuredBuffer) と、それに対応する CPU 上のデータ（std::vector）を保持・管理するクラス.
/// </summary>
/// <typeparam name="structBuff">要素となるデータ構造を定義したクラス/構造体</typeparam>
template <HasInConstantBuffer structBuff>
class IStructuredBuffer {
public:
    /// <summary>GPU側に送る実際の構造体定義</summary>
    using StructuredBufferType = typename structBuff::ConstantBuffer;

    IStructuredBuffer()  = default;
    ~IStructuredBuffer() = default;

    /// <summary>
    /// 指定された要素数でバッファを作成し、SRV/UAVを設定する.
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="elementCount">バッファの最大要素数</param>
    /// <param name="_withUAV">書き込み用 (UAV)ディスクリプタを作成するかどうか</param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount, bool _withUAV = false);

    /// <summary>
    /// バッファとディスクリプタ情報を解放する.
    /// </summary>
    void Finalize();

    /// <summary>CPU側で操作可能な生データのリスト（std::vector）</summary>
    std::vector<structBuff> openData_;

protected:
    /// <summary>D3D12リソース管理オブジェクト</summary>
    DxResource buff_;
    /// <summary>シェーダーリソースビュー (SRV) ディスクリプタ</summary>
    DxSrvDescriptor srv_;

    /// <summary>GPUバッファに対応するマッピングされたアドレス</summary>
    StructuredBufferType* mappingData_ = nullptr;

    /// <summary>構築されたバッファの要素数</summary>
    uint32_t elementCount_ = 0;

public:
    /// <summary>
    /// CPU 上の vector (openData_) の内容を GPU 用の構造化バッファに転送（コピー）する.
    /// </summary>
    void ConvertToBuffer();

    /// <summary>
    /// ルートパラメータ（記述子テーブル）に対して、この構造化バッファの SRV を紐付ける.
    /// </summary>
    /// <param name="cmdList">コマンドリスト</param>
    /// <param name="rootParameterNum">バインド先のルートパラメータ番号</param>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

    /// <summary>
    /// バッファの要素数を変更し、GPUリソースを再生成する. 既存のデータは破棄される.
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="newElementCount">新しい最大要素数</param>
    void Resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount);

    /// <summary>
    /// 現在の openData_ のサイズに合わせて、GPUバッファをリサイズする（要素数が足りない場合のみ）.
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void ResizeForDataSize(Microsoft::WRL::ComPtr<ID3D12Device> device);

    /// <summary>
    /// 現在 CPU 上に保持しているデータの数を取得する.
    /// </summary>
    /// <returns>要素数</returns>
    size_t Size() const { return openData_.size(); }

    /// <summary>
    /// GPU バッファが確保済み（Resize可能な最大）のキャパシティを取得する.
    /// </summary>
    /// <returns>キャパシティ</returns>
    size_t Capacity() const { return elementCount_; }

    /// <summary>DxResource オブジェクトへの参照を取得する.</summary>
    DxResource& GetResource() { return buff_; }

    /// <summary>アサインされている SRV ディスクリプタを取得する.</summary>
    const DxSrvDescriptor& GetSrv() const { return srv_; }
};

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount, bool _withUAV) {
    elementCount_ = elementCount;

    // 要素数が0なら何もしない
    if (elementCount_ == 0) {
        return;
    }

    // サイズに合わせてバッファを作成
    size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
    if (_withUAV) { // UAV付きバッファを作成
        buff_.CreateUAVBuffer(device, bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_UPLOAD);
    } else { // 通常のバッファを作成
        buff_.CreateBufferResource(device, bufferSize);
    }
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    // vectorの容量を確保
    openData_.reserve(elementCount);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    viewDesc.Buffer.NumElements         = elementCount;
    viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

    SRVEntry viewEntry{&buff_, viewDesc};
    srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&viewEntry);
}

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::Finalize() {
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv_);
    buff_.Finalize();
}

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::Resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount) {
    // 要素数が変わらない、または0なら何もしない
    if (newElementCount == elementCount_ || newElementCount == 0) {
        return;
    }

    // 今のバッファを削除
    Finalize();

    { // 新しいバッファを作成
        elementCount_ = newElementCount;

        size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
        buff_.CreateBufferResource(device, bufferSize);
        buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Buffer.FirstElement        = 0;
        viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        viewDesc.Buffer.NumElements         = elementCount_;
        viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

        SRVEntry viewEntry{&buff_, viewDesc};
        srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&viewEntry);
    }
}

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ResizeForDataSize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // 要素数が変わらない、または0なら何もしない
    // 要素数は openData_ のサイズに合わせる
    int32_t newElementCount = static_cast<int32_t>(openData_.size());
    if (newElementCount == elementCount_ || newElementCount == 0) {
        return;
    }

    // 今のバッファを削除
    Finalize();

    { // 新しいバッファを作成
        elementCount_ = newElementCount;

        size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
        buff_.CreateBufferResource(device, bufferSize);
        buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Buffer.FirstElement        = 0;
        viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        viewDesc.Buffer.NumElements         = elementCount_;
        viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

        SRVEntry viewEntry{&buff_, viewDesc};
        srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&viewEntry);
    }
}

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ConvertToBuffer() {
    if (mappingData_) {
        std::copy(openData_.begin(), openData_.end(), mappingData_);
    }
}

template <HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootDescriptorTable(rootParameterNum, srv_.GetGpuHandle());
}

} // namespace OriGine
