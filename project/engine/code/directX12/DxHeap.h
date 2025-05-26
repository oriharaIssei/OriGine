#pragma once

#include <wrl.h>

#include <vector>

#include <d3d12.h>

class DxHeap{
public:
	static DxHeap *getInstance();

	static const UINT rtvHeapSize = 8;
	static const UINT srvHeapSize = 514;
	static const UINT dsvHeapSize = 1;
public:
	void Initialize(ID3D12Device *device);

	void CompactRtvHeap(ID3D12Device *device,std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE,uint32_t>> &usedDescriptorsArrayss);
	void CompactSrvHeap(ID3D12Device *device,std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE,uint32_t>> &usedDescriptorsArrays);
	void CompactDsvHeap(ID3D12Device *device,std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE,uint32_t>> &usedDescriptorsArrays);

	void Finalize();
private:
	DxHeap() = default;
	~DxHeap() = default;
	DxHeap(const DxHeap &) = delete;
	DxHeap &operator=(const DxHeap &) = delete;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap(ID3D12Device *device,D3D12_DESCRIPTOR_HEAP_TYPE heapType,UINT numDescriptors,bool shaderVisible);
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;

	UINT rtvIncrementSize_;
	UINT dsvIncrementSize_;
	UINT srvIncrementSize_;
public:
	ID3D12DescriptorHeap *getRtvHeap()const{ return rtvHeap_.Get(); }
	ID3D12DescriptorHeap *getSrvHeap()const{ return srvHeap_.Get(); }
	ID3D12DescriptorHeap *getDsvHeap()const{ return dsvHeap_.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE getRtvCpuHandle(UINT index)const{ return D3D12_CPU_DESCRIPTOR_HANDLE(index * rtvIncrementSize_ + rtvHeap_->GetCPUDescriptorHandleForHeapStart().ptr); }
	D3D12_GPU_DESCRIPTOR_HANDLE getRtvGpuHandle(UINT index)const{ return D3D12_GPU_DESCRIPTOR_HANDLE(index * rtvIncrementSize_ + rtvHeap_->GetGPUDescriptorHandleForHeapStart().ptr); }

	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle(UINT index)const{ return D3D12_CPU_DESCRIPTOR_HANDLE(index * srvIncrementSize_ + srvHeap_->GetCPUDescriptorHandleForHeapStart().ptr); }
	D3D12_GPU_DESCRIPTOR_HANDLE getSrvGpuHandle(UINT index)const{ return D3D12_GPU_DESCRIPTOR_HANDLE(index * srvIncrementSize_ + srvHeap_->GetGPUDescriptorHandleForHeapStart().ptr); }

	D3D12_CPU_DESCRIPTOR_HANDLE getDsvCpuHandle(UINT index)const{ return D3D12_CPU_DESCRIPTOR_HANDLE(index * dsvIncrementSize_ + dsvHeap_->GetCPUDescriptorHandleForHeapStart().ptr); }
	D3D12_GPU_DESCRIPTOR_HANDLE getDsvGpuHandle(UINT index)const{ return D3D12_GPU_DESCRIPTOR_HANDLE(index * dsvIncrementSize_ + dsvHeap_->GetGPUDescriptorHandleForHeapStart().ptr); }
};
