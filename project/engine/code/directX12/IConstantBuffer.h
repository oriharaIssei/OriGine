#pragma once

/// stl & api
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

/// engine
// directX12
#include "directX12/DxResource.h"

/// lib
#include <logger/Logger.h>

template <typename T>
concept HasInConstantBuffer = requires {
    typename T::ConstantBuffer;
    requires std::is_copy_assignable_v<typename T::ConstantBuffer>;
    { std::declval<typename T::ConstantBuffer>() = std::declval<const T&>() } -> std::same_as<typename T::ConstantBuffer&>;
};

template <HasInConstantBuffer constBuff>
class IConstantBuffer {
public:
    template <typename... Args>
    IConstantBuffer(Args... args) {
        openData_ = constBuff(args...);
    }

    IConstantBuffer() {}
    ~IConstantBuffer() {}

    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void Finalize() { buff_.Finalize(); }

    void ConvertToBuffer() const;
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

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
    const DxResource& getResource() const { return buff_; }
};

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // リソースがすでに存在する場合は何もしない
    if (buff_.getResourceRef().Get()) {
        LOG_WARN("IConstantBuffer: Buffer resource already exists. Skipping creation.");
        return;
    }
    buff_.CreateBufferResource(device, sizeof(constBuff::ConstantBuffer));
    buff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    buff_.setType(DxResourceType::Buffer_Constant);
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::ConvertToBuffer() const {
    *mappingData_ = openData_;
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootConstantBufferView(rootParameterNum, buff_.getResource()->GetGPUVirtualAddress());
}
