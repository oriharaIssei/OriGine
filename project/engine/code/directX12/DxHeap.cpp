#include "DxHeap.h"

/// stl
#include <cassert>
// container
#include <vector>

/// lib
#include "logger/Logger.h"

DxHeap* DxHeap::getInstance() {
    static DxHeap instance = DxHeap();
    return &instance;
}

void DxHeap::Initialize(ID3D12Device* device) {
    ///================================================
    ///	Heap の生成
    ///================================================
    LOG_DEBUG("Initialize DxHeap");
    rtvHeap_ = CreateHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvHeapSize, false);
    srvHeap_ = CreateHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvHeapSize, true);
    dsvHeap_ = CreateHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvHeapSize, false);

    rtvIncrementSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    srvIncrementSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dsvIncrementSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    LOG_DEBUG("RtvHeap IncrementSize : " + std::to_string(rtvIncrementSize_));
    LOG_DEBUG("SrvHeap IncrementSize : " + std::to_string(srvIncrementSize_));
    LOG_DEBUG("DsvHeap IncrementSize : " + std::to_string(dsvIncrementSize_));
}

void DxHeap::CompactRtvHeap(ID3D12Device* device, std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, uint32_t>>& usedDescriptorsArrays) {
    LOG_DEBUG("Run Compact Rtv Heap Function");

    // RTV ヒープからハンドルを取得
    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();

    // 前詰め（左詰め）想定：先頭から順にコピー
    for (size_t i = 0; i < usedDescriptorsArrays.size(); ++i) {
        // コピー元とコピー先が重なる場合は、CopyDescriptorsSimpleをスキップ
        if (usedDescriptorsArrays[i].first.ptr != dstHandle.ptr) {
            // コピー範囲が重複していないかチェック
            SIZE_T srcStart = usedDescriptorsArrays[i].first.ptr;
            SIZE_T srcEnd   = srcStart + SIZE_T(rtvIncrementSize_) * usedDescriptorsArrays[i].second;
            SIZE_T dstStart = dstHandle.ptr;
            SIZE_T dstEnd   = dstStart + SIZE_T(rtvIncrementSize_) * usedDescriptorsArrays[i].second;
            // 範囲が重複していればスキップ（またはassert/エラー処理）
            if ((dstStart < srcEnd) && (srcStart < dstEnd)) {
                LOG_ERROR("CopyDescriptorsSimple: source and destination ranges overlap. Skipping copy.");
                // 必要に応じてassert(false);やcontinue;
                continue;
            }

            device->CopyDescriptorsSimple(
                usedDescriptorsArrays[i].second,
                dstHandle,
                usedDescriptorsArrays[i].first,
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        dstHandle.ptr += SIZE_T(rtvIncrementSize_) * usedDescriptorsArrays[i].second;
    }
}
void DxHeap::CompactSrvHeap(ID3D12Device* device, std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, uint32_t>>& usedDescriptorsArrays) {
    LOG_DEBUG("Run Compact Srv Heap Function");

    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();

    for (size_t i = 0; i < usedDescriptorsArrays.size(); ++i) {
        if (usedDescriptorsArrays[i].first.ptr != dstHandle.ptr) {
            SIZE_T srcStart = usedDescriptorsArrays[i].first.ptr;
            SIZE_T srcEnd   = srcStart + SIZE_T(srvIncrementSize_) * usedDescriptorsArrays[i].second;
            SIZE_T dstStart = dstHandle.ptr;
            SIZE_T dstEnd   = dstStart + SIZE_T(srvIncrementSize_) * usedDescriptorsArrays[i].second;
            if ((dstStart < srcEnd) && (srcStart < dstEnd)) {
                LOG_ERROR("CopyDescriptorsSimple: source and destination ranges overlap. Skipping copy.");
                continue;
            }

            device->CopyDescriptorsSimple(
                usedDescriptorsArrays[i].second,
                dstHandle,
                usedDescriptorsArrays[i].first,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        dstHandle.ptr += SIZE_T(srvIncrementSize_) * usedDescriptorsArrays[i].second;
    }
}
void DxHeap::CompactDsvHeap(ID3D12Device* device, std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, uint32_t>>& usedDescriptorsArrays) {
    LOG_DEBUG("Run Compact Dsv Heap Function");

    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();

    for (size_t i = 0; i < usedDescriptorsArrays.size(); ++i) {
        if (usedDescriptorsArrays[i].first.ptr != dstHandle.ptr) {
            SIZE_T srcStart = usedDescriptorsArrays[i].first.ptr;
            SIZE_T srcEnd   = srcStart + SIZE_T(dsvIncrementSize_) * usedDescriptorsArrays[i].second;
            SIZE_T dstStart = dstHandle.ptr;
            SIZE_T dstEnd   = dstStart + SIZE_T(dsvIncrementSize_) * usedDescriptorsArrays[i].second;
            if ((dstStart < srcEnd) && (srcStart < dstEnd)) {
                LOG_ERROR("CopyDescriptorsSimple: source and destination ranges overlap. Skipping copy.");
                continue;
            }

            device->CopyDescriptorsSimple(
                usedDescriptorsArrays[i].second,
                dstHandle,
                usedDescriptorsArrays[i].first,
                D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        }
        dstHandle.ptr += SIZE_T(dsvIncrementSize_) * usedDescriptorsArrays[i].second;
    }
}

void DxHeap::Finalize() {
    rtvHeap_.Reset();
    srvHeap_.Reset();
    dsvHeap_.Reset();
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DxHeap::CreateHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};

    rtvDesc.Type           = heapType;
    rtvDesc.NumDescriptors = numDescriptors;
    rtvDesc.Flags          = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(
        &rtvDesc,
        IID_PPV_ARGS(&descriptorHeap));

    if (SUCCEEDED(hr)) {
        LOG_DEBUG("Create DescriptorHeap\n Type : " + std::to_string(heapType) + "\n NumDescriptors : " + std::to_string(numDescriptors));
    } else {
        LOG_CRITICAL("Failed to create descriptor heap\n Type : " + std::to_string(heapType) + "\n NumDescriptors : " + std::to_string(numDescriptors));
        assert(false);
    }

    return descriptorHeap;
}
