#pragma once

/// stl
#include <vector>

/// engine
#include "Engine.h"
// directX12
#include "ConceptHasBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

namespace OriGine {
template <HasInConstantBuffer BufferType>
class AppendBuffer {
    constexpr static size_t defaultCapacity = 10000;

public:
    using BuffType = BufferType;

    AppendBuffer()  = default;
    ~AppendBuffer() = default;

    /// <summary>
    /// AppendBuffer用のバッファを作成する
    /// </summary>
    /// <param name="_device"></param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, size_t _capacity = defaultCapacity);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// Gpu 用バッファから openData_ に変換する
    /// </summary>
    /// <param name="_cmdList"></param>
    void ConvertFromBuffer(DxCommand* _dxCommand);

    /// <summary>
    /// ルートパラメータにAppendBufferをセット
    /// </summary>
    /// <param name="_cmdList"></param>
    /// <param name="_rootParameterNum"></param>
    void SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const;

private:
    DxUavDescriptor descriptor;
    DxResource buffer;

    BufferType* mappingData_ = nullptr;
    std::vector<BufferType> openData_;

    DxResource counterBuffer;

    size_t capacity_ = 0;

public:
    DxResource& GetBufferResource() { return buffer; }

    /// <summary>
    /// バッファの容量を取得
    /// </summary>
    /// <returns></returns>
    size_t GetCapacity() const { return capacity_; }

    /// <summary>
    /// 開かれたデータを取得
    /// </summary>
    /// <returns></returns>
    const std::vector<BufferType>& GetOpenData() const { return openData_; }
};

template <HasInConstantBuffer BufferType>
inline void AppendBuffer<BufferType>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> _device, size_t _capacity) {
    capacity_ = _capacity;
    // バッファリソースの作成
    size_t bufferSize = sizeof(BufferType) * capacity_;
    buffer.CreateUAVBuffer(_device, bufferSize);
    // リソースタイプの設定
    buffer.SetType(DxResourceType::Descriptor_UAV);

    // UAV ディスクリプタの作成
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format                      = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement         = 0;
    uavDesc.Buffer.NumElements          = static_cast<UINT>(capacity_);
    uavDesc.Buffer.StructureByteStride  = sizeof(BufferType);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

    // カウンターバッファの作成
    counterBuffer.CreateUAVBuffer(
        _device,
        sizeof(UINT),
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_HEAP_TYPE_DEFAULT);

    UAVEntry entry{&buffer, &counterBuffer, uavDesc};
    descriptor = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&entry);
}

template <HasInConstantBuffer BufferType>
inline void AppendBuffer<BufferType>::Finalize() {
    buffer.Finalize();
    mappingData_ = nullptr;
    openData_.clear();
    counterBuffer.Finalize();
}

template <HasInConstantBuffer BufferType>
inline void AppendBuffer<BufferType>::ConvertFromBuffer(DxCommand* _dxCommand) {
    auto& cmdList = _dxCommand->GetCommandList();
    auto& device  = Engine::GetInstance()->GetDxDevice()->device_;

    // -----------------------------
    // Counter Readback
    // -----------------------------
    DxResource readBackCounterBuffer;
    readBackCounterBuffer.CreateBufferResource(
        device,
        sizeof(UINT),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_HEAP_TYPE_READBACK // ★重要
    );

    cmdList->CopyResource(
        readBackCounterBuffer.GetResource().Get(),
        counterBuffer.GetResource().Get());

    _dxCommand->Close();
    _dxCommand->ExecuteCommandAndWait();
    _dxCommand->CommandReset();

    UINT elementCount = 0;
    {
        UINT* mapped = nullptr;
        D3D12_RANGE range{0, sizeof(UINT)};
        readBackCounterBuffer.GetResource()->Map(0, &range, reinterpret_cast<void**>(&mapped));
        elementCount = *mapped;
        readBackCounterBuffer.GetResource()->Unmap(0, nullptr);
    }

    // -----------------------------
    // Data Readback
    // -----------------------------
    if (elementCount > 0) {
        DxResource readBackBuffer;
        readBackBuffer.CreateBufferResource(
            device,
            sizeof(BufferType) * elementCount,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_HEAP_TYPE_READBACK // ★重要
        );

        UINT64 copySize = sizeof(BufferType) * elementCount;

        cmdList->CopyBufferRegion(
            readBackBuffer.GetResource().Get(), // dst
            0, // dstOffset
            buffer.GetResource().Get(), // src
            0, // srcOffset
            copySize // size
        );

        _dxCommand->Close();
        _dxCommand->ExecuteCommandAndWait();
        _dxCommand->CommandReset();

        BufferType* mappedData = nullptr;
        D3D12_RANGE range{0, sizeof(BufferType) * elementCount};
        readBackBuffer.GetResource()->Map(0, &range, reinterpret_cast<void**>(&mappedData));

        openData_.resize(elementCount);
        std::memcpy(openData_.data(), mappedData, sizeof(BufferType) * elementCount);

        readBackBuffer.GetResource()->Unmap(0, nullptr);
    }
}

template <HasInConstantBuffer BufferType>
inline void AppendBuffer<BufferType>::SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) const {
    // バッファのUAVをセット
    _cmdList->SetComputeRootDescriptorTable(_rootParameterNum, descriptor.GetGpuHandle());
}

} // namespace OriGine
