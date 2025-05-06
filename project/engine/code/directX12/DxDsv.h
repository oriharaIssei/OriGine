#pragma once

#include <wrl.h>

#include <d3d12.h>

class DxDepthStencilView {
public:
    void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* dsvHeap, UINT64 width, UINT height);
    void Finalize();

    void Resize(ID3D12Device* device, ID3D12DescriptorHeap* dsvHeap, UINT64 newWidth, UINT newHeight);

private:
    UINT64 width_ = 0;
    UINT height_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> dsv_;

public:
    ID3D12Resource* GetDepthStencil() const { return dsv_.Get(); }
};

using DxDsv = DxDepthStencilView;
