#pragma once

/// Engine
// directX12
#include "directX12/buffer/ConceptHasBuffer.h"

namespace OriGine {

/// <summary>
/// IStructuredBufferのOpenDataが無いバージョン
/// Bufferに入れる内容を外部で管理するためのバージョン
/// </summary>
template <HasInConstantBuffer structBuff>
class SimpleStructuredBuffer {
public:
    using StructuredBufferType = typename structBuff::ConstantBuffer;

    SimpleStructuredBuffer()  = default;
    ~SimpleStructuredBuffer() = default;

    /// <summary>
    /// Bufferを生成
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_elementCount">生成するbufferの数</param>
    /// <param name="_withUAV">uav付きか. true = UAV付き,false = UAVなし</param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, uint32_t _elementCount, bool _withUAV = false);

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

protected:
    DxResource buff_;
    std::shared_ptr<DxSrvDescriptor> srv_;

    // bind されたデータへのポインタ
    StructuredBufferType* mappingData_ = nullptr;

    uint32_t elementCount_ = 0;

public:
    /// <summary>
    /// データをBufferに変換してセット
    /// </summary>
    /// <param name="_inputData">入力データ</param>
    void ConvertToBuffer(std::vector<structBuff>& _inputData);

    /// <summary>
    /// ルートパラメータにセット
    /// </summary>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_rootParameterNum">パラメータ番号</param>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const;

    /// <summary>
    /// バッファのリサイズ
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_newElementCount">新しい要素数</param>
    void resize(Microsoft::WRL::ComPtr<ID3D12Device> _device, uint32_t _newElementCount);

    /// <summary>
    /// キャパシティの取得
    /// </summary>
    /// <returns>キャパシティ</returns>
    size_t capacity() const { return elementCount_; }

    /// <summary>
    /// リソースの取得
    /// </summary>
    /// <returns>リソース</returns>
    DxResource& GetResource() { return buff_; }

    /// <summary>
    /// SRVの取得
    /// </summary>
    /// <returns>SRV</returns>
    std::shared_ptr<DxSrvDescriptor> GetSrv() const { return srv_; }
};

template <HasInConstantBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, uint32_t _elementCount, bool _withUAV) {
    elementCount_ = _elementCount;

    if (elementCount_ == 0) {
        return;
    }

    size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
    if (_withUAV) {
        buff_.CreateUAVBuffer(_device, bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_UPLOAD);
    } else {
        buff_.CreateBufferResource(_device, bufferSize);
    }
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    viewDesc.Buffer.NumElements         = _elementCount;
    viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

    srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
}

template <HasInConstantBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::Finalize() {
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv_);
    buff_.Finalize();
}

template <HasInConstantBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::resize(Microsoft::WRL::ComPtr<ID3D12Device> _device, uint32_t _newElementCount) {
    if (_newElementCount == elementCount_ || _newElementCount == 0) {
        return;
    }

    // 今のバッファを削除
    Finalize();

    { // 新しいバッファを作成
        elementCount_ = _newElementCount;

        size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
        buff_.CreateBufferResource(_device, bufferSize);
        buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Buffer.FirstElement        = 0;
        viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        viewDesc.Buffer.NumElements         = elementCount_;
        viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

        srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
    }
}

template <HasInConstantBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::ConvertToBuffer(std::vector<structBuff>& _inputData) {
    if (mappingData_) {
        std::copy(_inputData.begin(), _inputData.end(), mappingData_);
    }
}

template <HasInConstantBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const {
    _cmdList->SetGraphicsRootDescriptorTable(_rootParameterNum, srv_->GetGpuHandle());
}

} // namespace OriGine
