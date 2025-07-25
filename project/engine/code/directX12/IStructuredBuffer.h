#pragma once

/// stl
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

/// engine
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"
#include "Engine.h"

template <typename T>
concept StructuredBuffer = requires {
    typename T::ConstantBuffer;
    requires std::is_copy_assignable_v<typename T::ConstantBuffer>;
    { std::declval<typename T::ConstantBuffer>() = std::declval<const T&>() } -> std::same_as<typename T::ConstantBuffer&>;
};
template <StructuredBuffer structBuff>
class IStructuredBuffer {
public:
    IStructuredBuffer()  = default;
    ~IStructuredBuffer() = default;

    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount);
    void Finalize();

    // 公開用変数（バッファのデータを保持）
    std::vector<structBuff> openData_;

protected:
    DxResource buff_;
    std::shared_ptr<DxSrvDescriptor> srv_;

    // bind されたデータへのポインタ
    structBuff::ConstantBuffer* mappingData_ = nullptr;

    uint32_t elementCount_ = 0;

public:
    void ConvertToBuffer();
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

    void resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount);
    void resizeForDataSize(Microsoft::WRL::ComPtr<ID3D12Device> device);
    size_t size() const { return openData_.size(); }
    size_t capacity() const { return elementCount_; }

    DxResource& getResource() { return buff_; }
    std::shared_ptr<DxSrvDescriptor> getSrv() const { return srv_; }
};

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t elementCount) {
    elementCount_ = elementCount;

    if (elementCount_ == 0) {
        return;
    }

    size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
    buff_.CreateBufferResource(device, bufferSize);
    buff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    openData_.reserve(elementCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    viewDesc.Buffer.NumElements         = elementCount;
    viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

    srv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
}

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::Finalize() {
    Engine::getInstance()->getSrvHeap()->ReleaseDescriptor(srv_);
    buff_.Finalize();
}

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::resize(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t newElementCount) {
    if (newElementCount == elementCount_ || newElementCount == 0) {
        return;
    }

    // 今のバッファを削除
    Finalize();

    { // 新しいバッファを作成
        elementCount_ = newElementCount;

        size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
        buff_.CreateBufferResource(device, bufferSize);
        buff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Buffer.FirstElement        = 0;
        viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        viewDesc.Buffer.NumElements         = elementCount_;
        viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

        srv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
    }
}

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::resizeForDataSize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    int32_t newElementCount = static_cast<int32_t>(openData_.size());
    if (newElementCount == elementCount_ || newElementCount == 0) {
        return;
    }

    // 今のバッファを削除
    Finalize();

    { // 新しいバッファを作成
        elementCount_ = newElementCount;

        size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
        buff_.CreateBufferResource(device, bufferSize);
        buff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Buffer.FirstElement        = 0;
        viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        viewDesc.Buffer.NumElements         = elementCount_;
        viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

        srv_ = Engine::getInstance()->getSrvHeap()->CreateDescriptor<>(viewDesc, &buff_);
    }
}

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ConvertToBuffer() {
    if (mappingData_) {
        std::copy(openData_.begin(), openData_.end(), mappingData_);
    }
}

template <StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootDescriptorTable(rootParameterNum, srv_->getGpuHandle());
}
