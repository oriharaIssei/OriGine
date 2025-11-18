#pragma once

/// Engine
// directX12
#include "directX12/buffer/IStructuredBuffer.h"

template <StructuredBuffer structBuff>
class SimpleStructuredBuffer {
public:
    using StructuredBufferType = typename structBuff::ConstantBuffer;

    SimpleStructuredBuffer()  = default;
    ~SimpleStructuredBuffer() = default;

    /// <summary>
    /// Bufferを生成
    /// </summary>
    /// <param name="device"></param>
    /// <param name="elementCount">生成するbufferの数</param>
    /// <param name="_withUAV">uav付きか. true = UAV付き,false = UAVなし</param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount, bool _withUAV = false);
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
    /// <param name="_inputData"></param>
    void ConvertToBuffer(std::vector<structBuff>& _inputData);
    /// <summary>
    /// ルートパラメータにセット
    /// </summary>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

    /// <summary>
    /// バッファのリサイズ
    /// </summary>
    void resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount);

    size_t capacity() const { return elementCount_; }

    DxResource& GetResource() { return buff_; }
    std::shared_ptr<DxSrvDescriptor> GetSrv() const { return srv_; }
};

template <StructuredBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount, bool _withUAV) {
    elementCount_ = elementCount;

    if (elementCount_ == 0) {
        return;
    }

    size_t bufferSize = sizeof(StructuredBufferType) * elementCount_;
    if (_withUAV) {
        buff_.CreateUAVBuffer(device, bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_UPLOAD);
    } else {
        buff_.CreateBufferResource(device, bufferSize);
    }
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    viewDesc.Buffer.NumElements         = elementCount;
    viewDesc.Buffer.StructureByteStride = sizeof(StructuredBufferType);

    srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
}

template <StructuredBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::Finalize() {
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv_);
    buff_.Finalize();
}

template <StructuredBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount) {
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

        srv_ = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
    }
}

template <StructuredBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::ConvertToBuffer(std::vector<structBuff>& _inputData) {
    if (mappingData_) {
        std::copy(_inputData.begin(), _inputData.end(), mappingData_);
    }
}

template <StructuredBuffer structBuff>
inline void SimpleStructuredBuffer<structBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootDescriptorTable(rootParameterNum, srv_->GetGpuHandle());
}
