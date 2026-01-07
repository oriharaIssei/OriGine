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
#include <directX12/DxResource.h>
// util
#include <util/BitArray.h>

namespace OriGine {

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
/// Descriptor 1つ分の生成情報を表すインターフェースクラス
/// </summary>
class IDescriptorEntry {
public:
    virtual ~IDescriptorEntry() = default;

    virtual DxDescriptorHeapType GetHeapType() const = 0;
    virtual void Create(
        ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const = 0;
};

/// <summary>
/// RTV Descriptor 1つ分の生成情報を表すクラス
/// </summary>
class RTVEntry final
    : public IDescriptorEntry {
public:
    RTVEntry(
        DxResource* resource,
        const D3D12_RENDER_TARGET_VIEW_DESC& desc)
        : resource_(resource), desc_(desc) {}
    DxDescriptorHeapType GetHeapType() const override {
        return DxDescriptorHeapType::RTV;
    }
    void Create(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const override {
        device->CreateRenderTargetView(
            resource_->GetResource().Get(),
            &desc_,
            handle);
        resource_->AddType(DxResourceType::Descriptor_RTV);
    }

private:
    DxResource* resource_;
    D3D12_RENDER_TARGET_VIEW_DESC desc_;
};

/// <summary>
/// SRV Descriptor 1つ分の生成情報を表すクラス
/// </summary>
class SRVEntry final
    : public IDescriptorEntry {
public:
    SRVEntry(
        DxResource* resource,
        const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
        : resource_(resource), desc_(desc) {}

    DxDescriptorHeapType GetHeapType() const override {
        return DxDescriptorHeapType::CBV_SRV_UAV;
    }

    void Create(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const override {
        device->CreateShaderResourceView(
            resource_->GetResource().Get(),
            &desc_,
            handle);
        resource_->AddType(DxResourceType::Descriptor_SRV);
    }

private:
    DxResource* resource_;
    D3D12_SHADER_RESOURCE_VIEW_DESC desc_;
};

/// <summary>
/// DSV Descriptor 1つ分の生成情報を表すクラス
/// </summary>
class DSVEntry final
    : public IDescriptorEntry {
public:
    DSVEntry(
        DxResource* resource,
        const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
        : resource_(resource), desc_(desc) {}
    DxDescriptorHeapType GetHeapType() const override {
        return DxDescriptorHeapType::DSV;
    }
    void Create(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const override {
        device->CreateDepthStencilView(
            resource_->GetResource().Get(),
            &desc_,
            handle);
        resource_->AddType(DxResourceType::Descriptor_DSV);
    }

private:
    DxResource* resource_;
    D3D12_DEPTH_STENCIL_VIEW_DESC desc_;
};

/// <summary>
/// UAV Descriptor 1つ分の生成情報を表すクラス
/// </summary>
class UAVEntry final
    : public IDescriptorEntry {
public:
    UAVEntry(
        DxResource* resource,
        DxResource* counter,
        const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
        : resource_(resource), counter_(counter), desc_(desc) {}

    DxDescriptorHeapType GetHeapType() const override {
        return DxDescriptorHeapType::CBV_SRV_UAV;
    }

    void Create(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const override {
        device->CreateUnorderedAccessView(
            resource_->GetResource().Get(),
            counter_ ? counter_->GetResource().Get() : nullptr,
            &desc_,
            handle);
        resource_->AddType(DxResourceType::Descriptor_UAV);
    }

private:
    DxResource* resource_;
    DxResource* counter_; // nullptr OK
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc_;
};

/// <summary>
/// Sampler Descriptor 1つ分の生成情報を表すクラス
/// </summary>
class SamplerEntry final
    : public IDescriptorEntry {
public:
    SamplerEntry(const D3D12_SAMPLER_DESC& desc)
        : desc_(desc) {}

    DxDescriptorHeapType GetHeapType() const override {
        return DxDescriptorHeapType::Sampler;
    }

    void Create(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const override {
        device->CreateSampler(&desc_, handle);
    }

private:
    D3D12_SAMPLER_DESC desc_;
};

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
        D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0)) : index(_index),
                                                                                   cpuHandle(_cpuHandle),
                                                                                   gpuHandle(_gpuHandle) {}
    ~DxDescriptor() {}

protected:
    uint32_t index                        = 0; // ヒープ内のインデックス
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0); // CPU側のハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0); // GPU側のハンドル
public:
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return cpuHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return gpuHandle; }

    uint32_t GetIndex() const { return index; }

    void SetCpuHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) { cpuHandle = handle; }
    void SetGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { gpuHandle = handle; }
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
    DescriptorType CreateDescriptor(IDescriptorEntry* _entry);

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
        usedFlags_.Set(index, true); // 使用中フラグをセット

        return descriptor;
    }

    /// <summary>
    /// Descriptorを解放する
    /// </summary>
    void ReleaseDescriptor(DescriptorType _descriptor) {
        // Descriptorが無効な場合は何もしない

        uint32_t index = _descriptor.GetIndex();
        // 使用中フラグをクリア
        usedFlags_.Set(index, false);
        // ヒープから削除
        descriptors_[index] = DescriptorType(0);
    }

protected:
    uint32_t Allocate() {
        // 空いているDescriptorを探す
        for (uint32_t i = 0; i < size_; ++i) {
            if (!usedFlags_.Get(i)) {
                // 空いているDescriptorを埋めて返す
                usedFlags_.Set(i, true);
                return i;
            }
        }
        LOG_ERROR("No available descriptors in DxDescriptorHeap");
        throw ::std::runtime_error("No available descriptors in DxDescriptorHeap");
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

    ::std::vector<DescriptorType> descriptors_;
    BitArray<> usedFlags_; // 使用中のフラグを管理するビット配列
public:
    uint32_t GetSize() const { return size_; }

    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetHeap() const { return heap_; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeapRef() { return heap_; }
    const Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice() const { return device_; }

    void SetDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { device_ = device; }

    DescriptorType GetDescriptor(uint32_t index) const {
        // インデックスが範囲外の場合は例外を投げる
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw ::std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        return descriptors_[index];
    }
    void SetDescriptor(uint32_t index, const DescriptorType& descriptor) {
        // インデックスが範囲外の場合は例外を投げる
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw ::std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        descriptors_[index] = descriptor;
        usedFlags_.Set(index, true);
    }
};

template <DxDescriptorHeapType Type>
inline void DxDescriptorHeap<Type>::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device) {
    // デバイスが無効な場合は例外を投げる
    if (!_device) {
        throw ::std::invalid_argument("Device cannot be null");
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
        LOG_ERROR("Device is not initialized \n Type : {}", DxResourceTypeToString(DxResourceType(Type)));
        return;
    }
    if (!heap_) {
        LOG_ERROR("Heap is not initialized \n Type : {}", DxResourceTypeToString(DxResourceType(Type)));
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

template <DxDescriptorHeapType Type>
inline DxDescriptorHeap<Type>::DescriptorType DxDescriptorHeap<Type>::CreateDescriptor(IDescriptorEntry* _entry) {
    if (_entry == nullptr || _entry->GetHeapType() != Type) {
        LOG_ERROR("HeapType does not match");
        return DescriptorType(0);
    }
    // Descriptorを割り当て
    DescriptorType descriptor = AllocateDescriptor();
    // Descriptorを割り当てた場所に作成
    _entry->Create(device_.Get(), descriptor.GetCpuHandle());
    return descriptor;
}

using DxRtvHeap = DxDescriptorHeap<DxDescriptorHeapType::RTV>;
using DxDsvHeap = DxDescriptorHeap<DxDescriptorHeapType::DSV>;
using DxSrvHeap = DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>;

} // namespace OriGine
