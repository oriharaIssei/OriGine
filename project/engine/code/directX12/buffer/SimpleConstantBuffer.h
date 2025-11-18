#pragma once

/// engine
// directX12
#include "IConstantBuffer.h"

/// <summary>
/// IConstantBufferのOpenDataが無いバージョン
/// Bufferに入れる内容を外部で管理するためのバージョン
/// </summary>
/// <typeparam name="constBuff"></typeparam>
template <HasInConstantBuffer constBuff>
class SimpleConstantBuffer {
public:
    SimpleConstantBuffer() {}
    ~SimpleConstantBuffer() {}

    /// <summary>
    /// Bufferの作成
    /// </summary>
    /// <param name="device"></param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() { buff_.Finalize(); }

    /// <summary>
    /// データをBufferに変換してセット
    /// </summary>
    void ConvertToBuffer(const constBuff& _inputData) const;
    /// <summary>
    /// RootParameterにセット (Graphics)
    /// </summary>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;
    /// <summary>
    /// RootParameterにセット (Compute)
    /// </summary>
    void SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

protected:
    // bind されたデータ
    constBuff::ConstantBuffer* mappingData_ = nullptr;
    DxResource buff_;

public:
    const DxResource& GetResource() const { return buff_; }
};

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // リソースがすでに存在する場合は何もしない
    if (buff_.GetResourceRef().Get()) {
        LOG_WARN("Buffer resource already exists. Skipping creation.");
        return;
    }

    buff_.CreateBufferResource(device, sizeof(constBuff::ConstantBuffer));
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    buff_.SetType(DxResourceType::Buffer_Constant);
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::ConvertToBuffer(const constBuff& _inputData) const {
    *mappingData_ = _inputData;
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetComputeRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}
