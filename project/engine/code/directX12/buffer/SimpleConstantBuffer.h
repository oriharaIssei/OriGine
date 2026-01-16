#pragma once

/// engine
// directX12
#include "IConstantBuffer.h"

namespace OriGine {

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
    /// <param name="_device">デバイス</param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() { buff_.Finalize(); }

    /// <summary>
    /// データをBufferに変換してセット
    /// </summary>
    /// <param name="_inputData">入力データ</param>
    void ConvertToBuffer(const constBuff& _inputData) const;
    /// <summary>
    /// RootParameterにセット (Graphics)
    /// </summary>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_rootParameterNum">パラメータ番号</param>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const;
    /// <summary>
    /// RootParameterにセット (Compute)
    /// </summary>
    /// <param name="_cmdList">コマンドリスト</param>
    /// <param name="_rootParameterNum">パラメータ番号</param>
    void SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const;

protected:
    // bind されたデータ
    constBuff::ConstantBuffer* mappingData_ = nullptr;
    DxResource buff_;

public:
    /// <summary>
    /// リソースの取得
    /// </summary>
    /// <returns>リソース</returns>
    const DxResource& GetResource() const { return buff_; }
};

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device) {
    // リソースがすでに存在する場合は何もしない
    if (buff_.GetResourceRef().Get()) {
        LOG_WARN("Buffer resource already exists. Skipping creation.");
        return;
    }

    buff_.CreateBufferResource(_device, sizeof(constBuff::ConstantBuffer));
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    buff_.SetType(DxResourceType::Buffer_Constant);
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::ConvertToBuffer(const constBuff& _inputData) const {
    *mappingData_ = _inputData;
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const {
    _cmdList->SetGraphicsRootConstantBufferView(_rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

template <HasInConstantBuffer constBuff>
inline void SimpleConstantBuffer<constBuff>::SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const {
    _cmdList->SetComputeRootConstantBufferView(_rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

} // namespace OriGine
