#include "DxDescriptor.h"

D3D12_DESCRIPTOR_HEAP_TYPE DxDescriptorTypeToD3D12HeapType(DxDescriptorHeapType type) {
    switch (type) {
    case DxDescriptorHeapType::RTV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case DxDescriptorHeapType::DSV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    case DxDescriptorHeapType::SRV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case DxDescriptorHeapType::UAV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case DxDescriptorHeapType::Sampler:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    default:
        throw std::invalid_argument("Unsupported descriptor type");
    }
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device>device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
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
