#include "DxDescriptor.h"

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> OriGine::CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};

    rtvDesc.Type           = _heapType;
    rtvDesc.NumDescriptors = _numDescriptors;
    rtvDesc.Flags          = _shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = _device->CreateDescriptorHeap(
        &rtvDesc,
        IID_PPV_ARGS(&descriptorHeap));

    if (SUCCEEDED(hr)) {
        LOG_DEBUG("Create DescriptorHeap\n Type : {} \n NumDescriptors : {}", std::to_string(_heapType), std::to_string(_numDescriptors));
    } else {
        LOG_CRITICAL("Failed to create descriptor heap\n Type : {} \n NumDescriptors : {}", std::to_string(_heapType), std::to_string(_numDescriptors));
        assert(false);
    }

    return descriptorHeap;
}
