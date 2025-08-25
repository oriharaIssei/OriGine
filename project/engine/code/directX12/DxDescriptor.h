#pragma once

/// api
#include <d3d12.h>
#include <wrl.h>

/// stl
#include <stdexcept>
#include <vector>

/// engine
#include <logger/Logger.h>
// directX12
#include <directX12/DxDevice.h>
#include <directX12/DxResource.h>
// util
#include <util/BitArray.h>
#include <util/EnumBitMask.h>

enum class DxDescriptorHeapType {
    CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, // Shader Resource View
    Sampler     = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, // Sampler
    RTV         = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, // Render Target View
    DSV         = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, // Depth Stencil View
};

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

template <DxDescriptorHeapType Type>
struct DxDescriptor {
    static constexpr DxDescriptorHeapType HeapType = Type;

    DxDescriptor(
        Microsoft::WRL::ComPtr<ID3D12Resource> _resource = nullptr,
        D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle           = 0,
        D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle           = 0) : resource_(_resource),
                                                      cpuHandle(_cpuHandle),
                                                      gpuHandle(_gpuHandle) {}
    ~DxDescriptor() {}

protected:
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle            = 0; // CPU側のハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle            = 0; // GPU側のハンドル
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr; // リソースへの参照
public:
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle() const { return cpuHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle() const { return gpuHandle; }
    Microsoft::WRL::ComPtr<ID3D12Resource> getResource() const { return resource_; }

    void setCpuHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) { cpuHandle = handle; }
    void setGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { gpuHandle = handle; }
    void setResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) { resource_ = resource; }
};

using DxRtvDescriptor     = DxDescriptor<DxDescriptorHeapType::RTV>;
using DxDsvDescriptor     = DxDescriptor<DxDescriptorHeapType::DSV>;
using DxSrvDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>;
using DxUavDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>; // cbv,srv,uav は 同一Heapで作成するので 一旦同じということにしておく
using DxSamplerDescriptor = DxDescriptor<DxDescriptorHeapType::Sampler>;

template <DxDescriptorHeapType Type>
class DxDescriptorHeap {
public:
    static const DxDescriptorHeapType DescriptorHeapType = Type;
    using DescriptorType                                 = DxDescriptor<DescriptorHeapType>;

    DxDescriptorHeap(uint32_t _size = 0) : size_(_size) {}
    ~DxDescriptorHeap() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device);
    void Finalize();

    // void CompactHeap();

    template <typename Desc>
    std::shared_ptr<DescriptorType> CreateDescriptor(const Desc& _desc, DxResource* _resource);

    std::shared_ptr<DescriptorType> AllocateDescriptor(DxResource* _resource) {
        if (!_resource) {
            LOG_ERROR("DxDescriptorHeap::AllocateDescriptor: Resource is null");
            throw std::invalid_argument("Resource is null");
        }

        uint32_t index                        = Allocate();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);

        std::shared_ptr<DescriptorType> descriptor = std::make_unique<DescriptorType>(_resource->getResource().Get(), cpuHandle, gpuHandle);

        usedFlags_.set(index, true); // 使用中フラグをセット

        return descriptor;
    }

    std::shared_ptr<DescriptorType> AllocateDescriptor(Microsoft::WRL::ComPtr<ID3D12Resource> _resource) {
        if (!_resource) {
            LOG_ERROR("DxDescriptorHeap::AllocateDescriptor: Resource is null");
            throw std::invalid_argument("Resource is null");
        }

        uint32_t index                        = Allocate();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);

        std::shared_ptr<DescriptorType> descriptor = std::make_unique<DescriptorType>(_resource.Get(), cpuHandle, gpuHandle);

        usedFlags_.set(index, true); // 使用中フラグをセット

        return descriptor;
    }

    std::shared_ptr<DescriptorType> AllocateDescriptor() {

        uint32_t index                        = Allocate();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);

        std::shared_ptr<DescriptorType> descriptor = std::make_unique<DescriptorType>(nullptr, cpuHandle, gpuHandle);

        usedFlags_.set(index, true); // 使用中フラグをセット

        return descriptor;
    }

    void ReleaseDescriptor(std::shared_ptr<DescriptorType> _descriptor) {
        if (!_descriptor) {
            LOG_ERROR("DxDescriptorHeap::ReleaseDescriptor: Descriptor is null");
            return;
        }

        uint32_t index = static_cast<uint32_t>((_descriptor->getCpuHandle().ptr - heap_->GetCPUDescriptorHandleForHeapStart().ptr) / descriptorIncrementSize_);

        usedFlags_.set(index, false); // 使用中フラグをクリア
        descriptors_[index].reset();
        descriptors_[index] = nullptr; // ヒープから削除

        _descriptor.reset();
    }

protected:
    uint32_t Allocate() {
        for (uint32_t i = 0; i < size_; ++i) {
            if (!usedFlags_.get(i)) {
                usedFlags_.set(i, true);
                return i;
            }
        }
        LOG_ERROR("No available descriptors in DxDescriptorHeap");
        throw std::runtime_error("No available descriptors in DxDescriptorHeap");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE CalculateCpuHandle(uint32_t index) const { return D3D12_CPU_DESCRIPTOR_HANDLE(index * descriptorIncrementSize_ + heap_->GetCPUDescriptorHandleForHeapStart().ptr); }
    D3D12_GPU_DESCRIPTOR_HANDLE CalculateGpuHandle(uint32_t index) const {
        if (!shaderVisible_) {
            return D3D12_GPU_DESCRIPTOR_HANDLE{0};
        }
        return D3D12_GPU_DESCRIPTOR_HANDLE(index * descriptorIncrementSize_ + heap_->GetGPUDescriptorHandleForHeapStart().ptr);
    }

protected:
    Microsoft::WRL::ComPtr<ID3D12Device> device_       = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;
    bool shaderVisible_                                = false;
    uint32_t size_                                     = 0; // ヒープのサイズ

    uint32_t descriptorIncrementSize_ = 0;

    std::vector<std::shared_ptr<DescriptorType>> descriptors_;
    BitArray<> usedFlags_; // 使用中のフラグを管理するビット配列
public:
    uint32_t getSize() const { return size_; }

    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& getHeap() const { return heap_; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> getHeapRef() { return heap_; }
    const Microsoft::WRL::ComPtr<ID3D12Device>& getDevice() const { return device_; }
    Microsoft::WRL::ComPtr<ID3D12Device> getDeviceRef() { return device_; }

    void setDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { device_ = device; }

    DescriptorType* getDescriptor(uint32_t index) const {
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        return descriptors_[index].get();
    }
    void setDescriptor(uint32_t index, const DescriptorType& descriptor) {
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        descriptors_[index] = std::make_shared<DescriptorType>(descriptor);
        usedFlags_.set(index, true);
    }
};

template <DxDescriptorHeapType Type>
inline void DxDescriptorHeap<Type>::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device) {
    if (!_device) {
        throw std::invalid_argument("Device cannot be null");
    }

    device_ = _device;

    D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE(Type);
    shaderVisible_                      = (Type == DxDescriptorHeapType::CBV_SRV_UAV || Type == DxDescriptorHeapType::Sampler);
    heap_                               = CreateHeap(device_.Get(), heapType, size_, shaderVisible_);

    descriptorIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapType);

    descriptors_.resize(size_);
    usedFlags_ = BitArray(size_);
}

template <DxDescriptorHeapType Type>
inline void DxDescriptorHeap<Type>::Finalize() {
    if (!device_) {
        LOG_ERROR("DxDescriptorHeap::Finalize: Device is not initialized \n Type : {}", DxResourceTypeToString(DxResourceType(Type)));
        return;
    }
    if (!heap_) {
        LOG_ERROR("DxDescriptorHeap::Finalize: Heap is not initialized \n Type : {}", DxResourceTypeToString(DxResourceType(Type)));
        return;
    }
    device_.Reset();
    heap_.Reset();

    size_                    = 0;
    descriptorIncrementSize_ = 0;
    shaderVisible_           = false;

    descriptors_.clear();
    usedFlags_ = 0;
}

using DxRtvHeap = DxDescriptorHeap<DxDescriptorHeapType::RTV>;
using DxDsvHeap = DxDescriptorHeap<DxDescriptorHeapType::DSV>;
using DxSrvHeap = DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>;

#pragma region "CreateDescriptor"
template <DxDescriptorHeapType Type>
template <typename Desc>
inline std::shared_ptr<typename DxDescriptorHeap<Type>::DescriptorType>
DxDescriptorHeap<Type>::CreateDescriptor(const Desc& _desc, DxResource* _resource) {
    LOG_CRITICAL("DxDescriptorHeap::CreateDescriptor: Not implemented for this type");
}

template <>
template <>
inline std::shared_ptr<typename DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType>
DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::CreateDescriptor(const D3D12_SHADER_RESOURCE_VIEW_DESC& _desc, DxResource* _resource) {
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }
    uint32_t index                        = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);
    device_->CreateShaderResourceView(_resource->getResource().Get(), &_desc, cpuHandle);
    auto descriptor = std::make_shared<DescriptorType>(_resource->getResource(), cpuHandle, gpuHandle);
    _resource->addType(DxResourceType::Descriptor_SRV); // リソースタイプを追加
    descriptors_[index] = descriptor; // ヒープに追加
    usedFlags_.set(index, true);
    return descriptor;
}

template <>
template <>
inline std::shared_ptr<typename DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType>
DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::CreateDescriptor(const D3D12_UNORDERED_ACCESS_VIEW_DESC& _desc, DxResource* _resource) {
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }
    uint32_t index                        = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);
    device_->CreateUnorderedAccessView(_resource->getResource().Get(), nullptr, &_desc, cpuHandle);
    auto descriptor = std::make_shared<DescriptorType>(_resource->getResource(), cpuHandle, gpuHandle);
    _resource->addType(DxResourceType::Descriptor_UAV); // リソースタイプを追加
    descriptors_[index] = descriptor; // ヒープに追加
    usedFlags_.set(index, true);
    return descriptor;
}

template <>
template <>
inline std::shared_ptr<typename DxDescriptorHeap<DxDescriptorHeapType::RTV>::DescriptorType>
DxDescriptorHeap<DxDescriptorHeapType::RTV>::CreateDescriptor(const D3D12_RENDER_TARGET_VIEW_DESC& _desc, DxResource* _resource) {
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }
    uint32_t index                        = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
    // D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);

    device_->CreateRenderTargetView(_resource->getResource().Get(), &_desc, cpuHandle);
    auto descriptor = std::make_shared<DescriptorType>(_resource->getResource(), cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE());
    _resource->addType(DxResourceType::Descriptor_RTV); // リソースタイプを追加
    descriptors_[index] = descriptor; // ヒープに追加
    usedFlags_.set(index, true);
    return descriptor;
}

template <>
template <>
inline std::shared_ptr<typename DxDescriptorHeap<DxDescriptorHeapType::DSV>::DescriptorType>
DxDescriptorHeap<DxDescriptorHeapType::DSV>::CreateDescriptor(const D3D12_DEPTH_STENCIL_VIEW_DESC& _desc, DxResource* _resource) {
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }
    uint32_t index                        = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
    // D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);
    device_->CreateDepthStencilView(_resource->getResource().Get(), &_desc, cpuHandle);
    auto descriptor = std::make_shared<DescriptorType>(_resource->getResource(), cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE());
    _resource->addType(DxResourceType::Descriptor_DSV); // リソースタイプを追加
    descriptors_[index] = descriptor; // ヒープに追加
    usedFlags_.set(index, true);
    return descriptor;
}

template <>
template <>
inline std::shared_ptr<typename DxDescriptorHeap<DxDescriptorHeapType::Sampler>::DescriptorType>
DxDescriptorHeap<DxDescriptorHeapType::Sampler>::CreateDescriptor(const D3D12_SAMPLER_DESC& _desc, DxResource* /*_resource*/) {
    uint32_t index                        = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);
    device_->CreateSampler(&_desc, cpuHandle);
    auto descriptor     = std::make_shared<DescriptorType>(nullptr, cpuHandle, gpuHandle);
    descriptors_[index] = descriptor; // ヒープに追加
    usedFlags_.set(index, true);
    return descriptor;
}

#pragma endregion
