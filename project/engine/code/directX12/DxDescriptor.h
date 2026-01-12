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

/// <summary>
/// ディスクリプタヒープの型を DirectX12 の列挙型と対応させた定義.
/// </summary>
enum class DxDescriptorHeapType {
    /// <summary>定数バッファ、シェーダーリソース、UAV用（Shader Visible可）</summary>
    CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    /// <summary>サンプラー用（Shader Visible可）</summary>
    Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
    /// <summary>レンダーターゲット用（Shader Visible不可）</summary>
    RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    /// <summary>深度ステンシルビュー用（Shader Visible不可）</summary>
    DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};

/// <summary>
/// DirectX12 のディスクリプタヒープを生成するためのヘルパー関数.
/// </summary>
/// <param name="device">使用するデバイス</param>
/// <param name="heapType">作成するヒープの種類</param>
/// <param name="numDescriptors">ヒープが保持するディスクリプタの数</param>
/// <param name="shaderVisible">シェーダーから直接参照可能（レジスタ経由）にするかどうか</param>
/// <returns>生成されたディスクリプタヒープ</returns>
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

/// <summary>
/// 特定のリソースに対してディスクリプタを生成するためのコマンドを抽象化したインターフェース.
/// </summary>
class IDescriptorEntry {
public:
    virtual ~IDescriptorEntry() = default;

    /// <summary>このエントリが必要とするヒープ種別を取得する.</summary>
    virtual DxDescriptorHeapType GetHeapType() const = 0;

    /// <summary>
    /// 指定されたハンドル位置に実際のディスクリプタを構築する.
    /// </summary>
    /// <param name="device">デバイス</param>
    /// <param name="handle">書き込み先のCPUディスクリプタハンドル</param>
    virtual void Create(
        ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE handle) const = 0;
};

/// <summary>
/// レンダーターゲットビュー (RTV) 生成情報を保持するエントリ.
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
/// シェーダーリソースビュー (SRV) 生成情報を保持するエントリ.
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
/// 深度ステンシルビュー (DSV) 生成情報を保持するエントリ.
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
/// 順序未指定アクセスビュー (UAV) 生成情報を保持するエントリ.
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
    DxResource* counter_; // カウンタリソース（任意）
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc_;
};

/// <summary>
/// サンプラー生成情報を保持するエントリ.
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
/// ヒープ内の単一の「場所」を指し、CPU/GPUハンドルをラップする構造体.
/// </summary>
/// <typeparam name="Type">対象とするヒープ種別</typeparam>
template <DxDescriptorHeapType Type>
struct DxDescriptor {
    static constexpr DxDescriptorHeapType HeapType = Type;

    /// <summary>
    /// デフォルトコンストラクタおよび初期化.
    /// </summary>
    /// <param name="_index">ヒープ内インデックス</param>
    /// <param name="_cpuHandle">CPU側ポインタ</param>
    /// <param name="_gpuHandle">GPU側ポインタ</param>
    DxDescriptor(
        uint32_t _index                        = 0,
        D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0),
        D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0)) : index(_index),
                                                                                   cpuHandle(_cpuHandle),
                                                                                   gpuHandle(_gpuHandle) {}
    ~DxDescriptor() {}

protected:
    /// <summary>ヒープ内のインデックス</summary>
    uint32_t index = 0;
    /// <summary>CPU側のハンドル</summary>
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(0);
    /// <summary>GPU側のハンドル（Shader不可視の場合は無効値）</summary>
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(0);

public:
    /// <summary>CPUハンドルを取得する.</summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return cpuHandle; }
    /// <summary>GPUハンドルを取得する.</summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return gpuHandle; }
    /// <summary>ヒープ内のインデックスを取得する.</summary>
    uint32_t GetIndex() const { return index; }

    /// <summary>CPUハンドルを直接設定する.</summary>
    void SetCpuHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) { cpuHandle = handle; }
    /// <summary>GPUハンドルを直接設定する.</summary>
    void SetGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { gpuHandle = handle; }
};

using DxRtvDescriptor     = DxDescriptor<DxDescriptorHeapType::RTV>;
using DxDsvDescriptor     = DxDescriptor<DxDescriptorHeapType::DSV>;
using DxSrvDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>;
using DxUavDescriptor     = DxDescriptor<DxDescriptorHeapType::CBV_SRV_UAV>; // cbv,srv,uav は 同一Heapで作成するので 一旦同じということにしておく
using DxSamplerDescriptor = DxDescriptor<DxDescriptorHeapType::Sampler>;

/// <summary>
/// ディスクリプタヒープを管理し、ディスクリプタの動的な割り当てと解放を行うクラス.
/// </summary>
/// <typeparam name="Type">ヒープの種類（RTV, DSV, CBV_SRV_UAV, Sampler）</typeparam>
template <DxDescriptorHeapType Type>
class DxDescriptorHeap {
public:
    static const DxDescriptorHeapType DescriptorHeapType = Type;
    using DescriptorType                                 = DxDescriptor<DescriptorHeapType>;

    /// <summary>
    /// コンストラクタ.
    /// </summary>
    /// <param name="_size">ヒープの最大ディスクリプタ数</param>
    DxDescriptorHeap(uint32_t _size = 0) : size_(_size) {}
    ~DxDescriptorHeap() = default;

    /// <summary>
    /// 指定されたデバイスを使用してヒープを初期化する.
    /// </summary>
    /// <param name="_device">D3D12デバイス</param>
    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device);

    /// <summary>
    /// ヒープとデバイス参照を解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 指定された生成情報（Entry）に基づいて、ヒープ内にディスクリプタを構築し、ラップオブジェクトを返す.
    /// </summary>
    /// <param name="_entry">生成情報ポインタ</param>
    /// <returns>割り当てられたディスクリプタ</returns>
    DescriptorType CreateDescriptor(IDescriptorEntry* _entry);

    /// <summary>
    /// 空いているディスクリプタの枠を一つ確保して返す（実際の作成は行わない）.
    /// </summary>
    /// <returns>割り当てられたディスクリプタ</returns>
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
    /// 指定されたディスクリプタを解放し、再利用可能な状態にする.
    /// </summary>
    /// <param name="_descriptor">解放するディスクリプタ</param>
    void ReleaseDescriptor(DescriptorType _descriptor) {
        uint32_t index = _descriptor.GetIndex();
        // 使用中フラグをクリア
        usedFlags_.Set(index, false);
        // ヒープから削除
        descriptors_[index] = DescriptorType(0);
    }

protected:
    /// <summary>
    /// ビット配列から空きインデックスを検索して確保する.
    /// </summary>
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

    /// <summary>インデックスからCPUハンドルを計算する.</summary>
    D3D12_CPU_DESCRIPTOR_HANDLE CalculateCpuHandle(uint32_t index) const { return D3D12_CPU_DESCRIPTOR_HANDLE(index * descriptorIncrementSize_ + heap_->GetCPUDescriptorHandleForHeapStart().ptr); }

    /// <summary>インデックスからGPUハンドルを計算する. Shader不可視の場合は無効値を返す.</summary>
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
    /// <summary>シェーダーから参照可能なヒープ（CBV/SRV/UAV/Sampler）かどうか</summary>
    bool shaderVisible_ = false;
    /// <summary>ヒープの最大要素数</summary>
    uint32_t size_ = 0;

    /// <summary>ディスクリプタ間のバイトサイズ（ハードウェア依存）</summary>
    uint32_t descriptorIncrementSize_ = 0;

    /// <summary>割り当て済みディスクリプタのキャッシュ</summary>
    ::std::vector<DescriptorType> descriptors_;
    /// <summary>各スロットの使用状況を管理するフラグ</summary>
    BitArray<> usedFlags_;

public:
    /// <summary>ヒープの最大サイズを取得する.</summary>
    uint32_t GetSize() const { return size_; }

    /// <summary>D3D12ヒープオブジェクトを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetHeap() const { return heap_; }
    /// <summary>D3D12ヒープオブジェクトを取得する（非const）.</summary>
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeapRef() { return heap_; }
    /// <summary>紐付いているデバイスを取得する.</summary>
    const Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice() const { return device_; }

    /// <summary>デバイスを明示的に設定する（Initialize 時に行われる）.</summary>
    void SetDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { device_ = device; }

    /// <summary>インデックス指定でディスクリプタを取得する.</summary>
    DescriptorType GetDescriptor(uint32_t index) const {
        if (index >= descriptors_.size()) {
            LOG_ERROR("Index out of range in DxDescriptorHeap");
            throw ::std::out_of_range("Index out of range in DxDescriptorHeap");
        }
        return descriptors_[index];
    }

    /// <summary>インデックス指定でディスクリプタを強制設定する.</summary>
    void SetDescriptor(uint32_t index, const DescriptorType& descriptor) {
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
