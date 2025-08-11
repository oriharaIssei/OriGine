#include "DxDescriptor.h"

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
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
        LOG_DEBUG("Create DescriptorHeap\n Type : {} \n NumDescriptors : {}", std::to_string(heapType), std::to_string(numDescriptors));
    } else {
        LOG_CRITICAL("Failed to create descriptor heap\n Type : {} \n NumDescriptors : {}", std::to_string(heapType), std::to_string(numDescriptors));
        assert(false);
    }

    return descriptorHeap;
}
