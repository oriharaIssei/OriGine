#pragma once

/// stl
#include <vector>

/// engine
#include "Engine.h"
// directX12
#include "ConceptHasBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/DxResource.h"

/// util
#include "util/nameof.h"

namespace OriGine {

/// <summary>
/// 読み書き可能な構造化バッファ
/// </summary>
template <HasInConstantBuffer T>
class RwStructuredBuffer {
public:
    using BufferType = T::ConstantBuffer;

    RwStructuredBuffer()  = default;
    ~RwStructuredBuffer() = default;

    /// <summary>
    /// Bufferを作成する
    /// </summary>
    /// <param name="_elementCount"></param>
    /// <param name="_device"></param>
    /// <param name="_uavHeap"></param>
    void CreateBuffer(size_t _elementCount, Microsoft::WRL::ComPtr<ID3D12Device> _device = Engine::GetInstance()->GetDxDevice()->device_, DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* _uavHeap = Engine::GetInstance()->GetSrvHeap());

    /// <summary>
    /// バッファの解放処理
    /// </summary>
    void Finalize(DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* _uavHeap = Engine::GetInstance()->GetSrvHeap());

    /// <summary>
    /// Gpu 用バッファから openData_ に変換する
    /// </summary>
    void ConvertFromBuffer(DxCommand* _dxCommand);

    /// <summary>
    /// ルートパラメーターにセット
    /// </summary>
    /// <param name="_commandList"></param>
    /// <param name="_rootParameterIndex"></param>
    void SetForComputePipeline(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, uint32_t _rootParameterIndex);

    /// <summary>
    /// GPUバッファの内容をReadbackバッファへコピーするコマンドを積む
    /// </summary>
    void CopyToReadback(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList);

    const std::vector<BufferType>& RetrieveData();

public:
    std::vector<BufferType> openData_ = {};

private:
    DxUavDescriptor descriptor_ = DxUavDescriptor();
    DxResource buffer_          = DxResource();
    DxResource readbackBuffer_  = DxResource();

    size_t elementCount_     = 0;
    BufferType* mappingData_ = nullptr;

public:
    const DxUavDescriptor& GetDescriptor() const {
        return descriptor_;
    }
    const DxResource& GetResource() const {
        return buffer_;
    }
    DxResource& GetResourceRef() {
        return buffer_;
    }
};

template <HasInConstantBuffer T>
inline void RwStructuredBuffer<T>::CreateBuffer(size_t _elementCount, Microsoft::WRL::ComPtr<ID3D12Device> _device, DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* _uavHeap) {
    if (_elementCount == 0) {
        return;
    }
    elementCount_ = _elementCount;

    size_t byteStride = sizeof(BufferType);

    // uav buffer 作成
    buffer_.CreateUAVBuffer(_device, byteStride * _elementCount);

    // readback buffer 作成
    readbackBuffer_.CreateBufferResource(_device, byteStride * _elementCount, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK);
    readbackBuffer_.SetName(L"RWStructuredBuffer_Readback");

    // UAVディスクリプタも再生成
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
    uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement         = 0;
    uavDesc.Buffer.NumElements          = static_cast<UINT>(_elementCount);
    uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
    uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
    uavDesc.Buffer.StructureByteStride  = static_cast<UINT>(byteStride);
    UAVEntry uavEntry(&buffer_, nullptr, uavDesc);
    descriptor_ = _uavHeap->CreateDescriptor(&uavEntry);
}

template <HasInConstantBuffer T>
inline void RwStructuredBuffer<T>::Finalize(DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* _uavHeap) {
    _uavHeap->ReleaseDescriptor(descriptor_);

    buffer_.Finalize();
    readbackBuffer_.Finalize();
}

template <HasInConstantBuffer T>
inline void RwStructuredBuffer<T>::ConvertFromBuffer(DxCommand* _dxCommand) {
    // GPUバッファの内容をReadbackバッファへコピーするコマンドを積む
    auto& commandList = _dxCommand->GetCommandList();
    CopyToReadback(commandList);

    // コマンドリストをクローズして実行
    _dxCommand->Close();
    _dxCommand->ExecuteCommandAndWait();
    _dxCommand->CommandReset();

    // コマンドリストをクローズして実行
    auto* dxFence = Engine::GetInstance()->GetDxFence();
    dxFence->WaitForFence(dxFence->Signal(_dxCommand->GetCommandQueue()));

    RetrieveData();
}

template <HasInConstantBuffer T>
inline void RwStructuredBuffer<T>::SetForComputePipeline(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList, uint32_t _rootParameterIndex) {
    _commandList->SetComputeRootDescriptorTable(_rootParameterIndex, descriptor_.GetGpuHandle());
}

template <HasInConstantBuffer T>
inline void RwStructuredBuffer<T>::CopyToReadback(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList) {
    // バリア: UAVアクセス -> コピー転送元
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = buffer_.GetResource().Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;

    // バリア発行
    _commandList->ResourceBarrier(1, &barrier);

    // コピー実行
    _commandList->CopyResource(readbackBuffer_.GetResource().Get(), buffer_.GetResource().Get());

    // バリア: コピー転送元 -> UAVアクセス (元の状態に戻す)
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    _commandList->ResourceBarrier(1, &barrier);
}

template <HasInConstantBuffer T>
inline const std::vector<typename RwStructuredBuffer<T>::BufferType>& RwStructuredBuffer<T>::RetrieveData() {
    // Readbackバッファをマップ
    BufferType* readbackData = nullptr;
    D3D12_RANGE readRange{0, elementCount_ * sizeof(BufferType)};

    HRESULT hr = readbackBuffer_.GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&readbackData));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to map readback buffer in RwStructuredBuffer::RetrieveData<{}>", nameof<BufferType>());
        throw std::runtime_error("Failed to map readback buffer in RwStructuredBuffer::RetrieveData");
    }

    // データをコピー
    openData_.resize(elementCount_);
    memcpy(openData_.data(), readbackData, elementCount_ * sizeof(BufferType));

    // アンマップ
    D3D12_RANGE writeRange{0, 0}; // 書き込みはしないので範囲は0
    readbackBuffer_.GetResource()->Unmap(0, &writeRange);

    return openData_;
}

} // namespace OriGine
