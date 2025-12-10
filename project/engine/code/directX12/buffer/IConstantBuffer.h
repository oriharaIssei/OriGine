#pragma once

/// stl & api
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

/// engine
// directX12
#include "directX12/DxResource.h"

/// util
#include <logger/Logger.h>

namespace OriGine {

/// <summary>
/// 内部宣言で ConstantBuffer 型を持つことを要求するコンセプト
/// </summary>
template <typename T>
concept HasInConstantBuffer = requires {
    typename T::ConstantBuffer;
    requires ::std::is_copy_assignable_v<typename T::ConstantBuffer>;
    { ::std::declval<typename T::ConstantBuffer>() = ::std::declval<const T&>() } -> ::std::same_as<typename T::ConstantBuffer&>;
};

/// <summary>
/// ConstantBufferと外部データを保持するクラス
/// </summary>
/// <typeparam name="constBuff"></typeparam>
template <HasInConstantBuffer constBuff>
class IConstantBuffer {
public:
    template <typename... Args>
    IConstantBuffer(Args... args) {
        openData_ = constBuff(args...);
    }

    IConstantBuffer() {}
    ~IConstantBuffer() {}

    /// <summary>
    /// ConstantBuffer用のバッファを作成する
    /// </summary>
    /// <param name="device"></param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
    /// <summary>
    /// 終了処理
    /// </summary>
    inline void Finalize();

    /// <summary>
    /// openData_ の内容を GPU 用バッファに変換する
    void ConvertToBuffer() const;
    /// <summary>
    /// グラフィックス用ルートパラメータにConstantBufferをセット
    /// </summary>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;
    /// <summary>
    /// コンピュート用ルートパラメータにConstantBufferをセット
    /// </summary>
    /// <param name="cmdList"></param>
    /// <param name="rootParameterNum"></param>
    void SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

public:
    // 公開用変数
    constBuff openData_;
    // openData_ のアクセス
    constBuff* operator->() { return &openData_; }
    const constBuff* operator->() const { return &openData_; }
    operator const constBuff*() const { return &openData_; }

protected:
    // bind されたデータ
    constBuff::ConstantBuffer* mappingData_ = nullptr;
    DxResource buff_;

public:
    const DxResource& GetResource() const { return buff_; }
};

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // リソースがすでに存在する場合は何もしない
    if (buff_.GetResourceRef().Get()) {
        LOG_WARN("IConstantBuffer: Buffer resource already exists. Skipping creation.");
        return;
    }

    // バッファリソースの作成
    buff_.CreateBufferResource(device, sizeof(constBuff::ConstantBuffer));
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    // リソースタイプの設定
    buff_.SetType(DxResourceType::Buffer_Constant);
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::Finalize() {
    // buffer が有効な場合は終了処理を行う
    if (buff_.IsValid()) {
        buff_.Finalize();
    }
}
template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::ConvertToBuffer() const {
    *mappingData_ = openData_;
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetComputeRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

} // namespace OriGine
