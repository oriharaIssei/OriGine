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

/// <summary>
/// Heapを作成する Helper関数
/// </summary>
/// <param name="device"></param>
/// <param name="heapType"></param>
/// <param name="numDescriptors">Heapが持てるDescriptorの数</param>
/// <param name="shaderVisible">shaderから参照可能なのかどうか. true = 参照可能/false = 参照不可能</param>
/// <returns></returns>
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

/// <summary>
/// Descriptor 1つを表すクラス
/// </summary>
/// <typeparam name="Type"></typeparam>
template <DxDescriptorHeapType Type>
struct DxDescriptor {
    static constexpr DxDescriptorHeapType HeapType = Type;

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="_index">Heap内での自身のインデックス</param>
    /// <param name="_resource">DescriptorのResource</param>
    /// <param name="_cpuHandle"></param>
    /// <param name="_gpuHandle"></param>
    DxDescriptor(
        uint32_t _index                        = 0,
        D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0),
        D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0)) : index_(_index),
                                                                                   cpuHandle(_cpuHandle),
                                                                                   gpuHandle(_gpuHandle) {}
    ~DxDescriptor() {}

protected:
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0); // CPU側のハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0); // GPU側のハンドル
    uint32_t index_                       = 0; // ヒープ内のインデックス
public:
    D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle() const { return cpuHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle() const { return gpuHandle; }

    uint32_t getIndex() const { return index_; }

    void setCpuHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) { cpuHandle = handle; }
    void setGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { gpuHandle = handle; }
};

using DxRtvDescriptor     = DxDescriptor<DxDescriptorHeapType::RTV>;
using DxDsvDescriptor     = DxDescriptor<DxDescriptorHeapType::DSV>;
using DxSrvDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>;
using DxUavDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>; // cbv,srv,uav は 同一Heapで作成するので 一旦同じということにしておく
using DxSamplerDescriptor = DxDescriptor<DxDescriptorHeapType::Sampler>;

/// <summary>
/// Descriptor Heapを表すクラス. Descriptorの生成、削除管理を行う
/// </summary>
template <DxDescriptorHeapType Type>
class DxDescriptorHeap {
public:
    static const DxDescriptorHeapType DescriptorHeapType = Type;
    using DescriptorType                                 = DxDescriptor<DescriptorHeapType>;

    DxDescriptorHeap(uint32_t _size = 0) : size_(_size) {}
    ~DxDescriptorHeap() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device);
    void Finalize();

    /// <summary>
    /// Descriptorを作成する
    /// </summary>
    /// <typeparam name="Desc"></typeparam>
    /// <param name="_desc"></param>
    /// <param name="_resource"></param>
    /// <returns></returns>
    template <typename Desc>
    DescriptorType CreateDescriptor(const Desc& _desc, DxResource* _resource);

    /// <summary>
    /// Descriptorを割り当てる
    /// </summary>
    /// <param name="_resource"></param>
    /// <returns></returns>
    DescriptorType AllocateDescriptor() {

        // Descriptorを割り当て
        uint32_t index                        = Allocate();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CalculateCpuHandle(index);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CalculateGpuHandle(index);

        // Descriptorを割り当てた場所に作成
        DescriptorType descriptor = DescriptorType(index, cpuHandle, gpuHandle);

        descriptors_[index] = descriptor;
        usedFlags_.set(index, true); // 使用中フラグをセット

        return descriptor;
    }

    /// <summary>
    /// Descriptorを解放する
    /// </summary>
    void ReleaseDescriptor(DescriptorType _descriptor) {
        // Descriptorが無効な場合は何もしない

        uint32_t index = _descriptor.getIndex();
        // 使用中フラグをクリア
        usedFlags_.set(index, false);
        // ヒープから削除
        descriptors_[index] = DescriptorType(0);
    }

protected:
    uint32_t Allocate() {
        // 空いているDescriptorを探す
        for (uint32_t i = 0; i < size_; ++i) {
            if (!usedFlags_.get(i)) {
                // 空いているDescriptorを埋めて返す
                usedFlags_.set(i, true);
                return i;
            }
        }
        LOG_ERROR("No available descriptors in DxDescriptorHeap");
        throw std::runtime_error("No available descriptors in DxDescriptorHeap");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE CalculateCpuHandle(uint32_t index) const { return D3D12_CPU_DESCRIPTOR_HANDLE(index * descriptorIncrementSize_ + heap_->GetCPUDescriptorHandleForHeapStart().ptr); }
    D3D12_GPU_DESCRIPTOR_HANDLE CalculateGpuHandle(uint32_t index) const {
        // Shaderから参照できないHeapの場合は0を返す
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

    std::vector<DescriptorType> descriptors_;
    BitArray<> usedFlags_; // 使用中のフラグを管理するビット配列
public:
    uint32_t getSize() const { return size_; }

    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& getHeap() const { return heap_; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> getHeapRef() { return heap_; }
    const Microsoft::WRL::ComPtr<ID3D12Device>& getDevice() const { return device_; }

    void setDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { device_ = device; }

    DescriptorType* getDescriptor(uint32_t index) const {
        // インデックスが範囲外の場合は例外を投げる
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        return descriptors_[index].get();
    }
    void setDescriptor(uint32_t index, const DescriptorType& descriptor) {
        // インデックスが範囲外の場合は例外を投げる
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
    // デバイスが無効な場合は例外を投げる
    if (!_device) {
        throw std::invalid_argument("Device cannot be null");
    }

    device_ = _device;

    // ヒープを作成
    D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE(Type);
    // シェーダーから参照可能かどうかを設定 (参照可能なのは CBV_SRV_UAV もしくは Samplerのみ)
    shaderVisible_ = (Type == DxDescriptorHeapType::CBV_SRV_UAV || Type == DxDescriptorHeapType::Sampler);
    heap_          = CreateHeap(device_.Get(), heapType, size_, shaderVisible_);

    // ディスクリプタのインクリメントサイズを取得
    descriptorIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapType);

    // ディスクリプタ配列と使用中フラグ配列を初期化
    descriptors_.resize(size_);
    usedFlags_ = BitArray(size_);
}

template <DxDescriptorHeapType Type>
inline void DxDescriptorHeap<Type>::Finalize() {
    // デバイスまたはヒープが無効な場合は何もしない
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
inline typename DxDescriptorHeap<Type>::DescriptorType
DxDescriptorHeap<Type>::CreateDescriptor(const Desc& _desc, DxResource* _resource) {
    // デフォルト実装はエラーを出す
    LOG_CRITICAL("DxDescriptorHeap::CreateDescriptor: Not implemented for this type");
}

template <>
template <>
inline typename DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType
DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::CreateDescriptor(const D3D12_SHADER_RESOURCE_VIEW_DESC& _desc, DxResource* _resource) {
    // リソースが無効な場合は例外を投げる
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }

    // Descriptorを割り当て & SRV作成
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType descriptor = AllocateDescriptor();
    device_->CreateShaderResourceView(_resource->getResource().Get(), &_desc, descriptor.getCpuHandle());

    // リソースタイプを追加
    _resource->addType(DxResourceType::Descriptor_SRV);

    return descriptor;
}

template <>
template <>
inline typename DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType
DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::CreateDescriptor(const D3D12_UNORDERED_ACCESS_VIEW_DESC& _desc, DxResource* _resource) {
    // リソースが無効な場合は例外を投げる
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }

    // Descriptorを割り当て & UAV作成
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>::DescriptorType descriptor = AllocateDescriptor();

    device_->CreateUnorderedAccessView(_resource->getResource().Get(), nullptr, &_desc, descriptor.getCpuHandle());

    // リソースタイプを追加
    _resource->addType(DxResourceType::Descriptor_UAV);

    return descriptor;
}

template <>
template <>
inline typename DxDescriptorHeap<DxDescriptorHeapType::RTV>::DescriptorType
DxDescriptorHeap<DxDescriptorHeapType::RTV>::CreateDescriptor(const D3D12_RENDER_TARGET_VIEW_DESC& _desc, DxResource* _resource) {
    // リソースが無効な場合は例外を投げる
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }

    // Descriptorを割り当て & RTV作成
    DxDescriptorHeap<DxDescriptorHeapType::RTV>::DescriptorType descriptor = AllocateDescriptor();
    device_->CreateRenderTargetView(_resource->getResource().Get(), &_desc, descriptor.getCpuHandle());

    // リソースタイプを追加
    _resource->addType(DxResourceType::Descriptor_RTV);

    return descriptor;
}

template <>
template <>
inline typename DxDescriptorHeap<DxDescriptorHeapType::DSV>::DescriptorType
DxDescriptorHeap<DxDescriptorHeapType::DSV>::CreateDescriptor(const D3D12_DEPTH_STENCIL_VIEW_DESC& _desc, DxResource* _resource) {
    // リソースが無効な場合は例外を投げる
    if (!_resource) {
        LOG_ERROR("DxDescriptorHeap::CreateDescriptor: Resource is null");
        throw std::invalid_argument("Resource is null");
    }

    // Descriptorを割り当て & DSV作成
    DxDescriptorHeap<DxDescriptorHeapType::DSV>::DescriptorType descriptor = AllocateDescriptor();

    device_->CreateDepthStencilView(_resource->getResource().Get(), &_desc, descriptor.getCpuHandle());

    // リソースタイプを追加
    _resource->addType(DxResourceType::Descriptor_DSV);

    return descriptor;
}

template <>
template <>
inline typename DxDescriptorHeap<DxDescriptorHeapType::Sampler>::DescriptorType
DxDescriptorHeap<DxDescriptorHeapType::Sampler>::CreateDescriptor(const D3D12_SAMPLER_DESC& _desc, DxResource* /*_resource*/) {
    // Descriptorを割り当て & Sampler作成
    DxDescriptorHeap<DxDescriptorHeapType::Sampler>::DescriptorType descriptor = AllocateDescriptor();

    // Samplerはリソースを持たないので nullptr を渡す
    device_->CreateSampler(&_desc, descriptor.getCpuHandle());

    return descriptor;
}

#pragma endregion
