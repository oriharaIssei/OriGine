#include "directX12/DxRtvArray.h"

void DxRtvArray::Finalize()
{
	for(auto& resource : resources_)
	{
		if(resource != nullptr)
		{
			resource.Reset();
		}
	}
}

uint32_t DxRtvArray::CreateView(ID3D12Device* device,D3D12_RENDER_TARGET_VIEW_DESC& viewDesc,const Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
	DxHeap* dxHeap = DxHeap::getInstance();

	uint32_t index = 0;
	for(; index < resources_.size(); index++)
	{
		if(resources_[index] != nullptr)
		{
			continue;
		}
		break;
	}
	device->CreateRenderTargetView(resource.Get(),&viewDesc,dxHeap->getRtvCpuHandle(arrayStartLocation_ + index));
	resources_[index] = resource;

	return index;
}

void DxRtvArray::DestroyView(uint32_t index)
{
	resources_[index].Reset();
	resources_[index] = nullptr;
}

void DxRtvArray::ClearTarget(uint32_t index,ID3D12GraphicsCommandList* commandList,const float* color)
{
	DxHeap* heap = DxHeap::getInstance();

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = heap->getDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtvHandle = heap->getRtvCpuHandle(getLocationOnHeap(index));
	commandList->OMSetRenderTargets(
		1,
		&backBufferRtvHandle,
		false,
		&dsvHandle
	);

	commandList->ClearRenderTargetView(
		backBufferRtvHandle,color,0,nullptr
	);

	commandList->ClearDepthStencilView(
		dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr
	);
}

void DxRtvArray::Init(uint32_t size,uint32_t arrayLocation)
{
	size_ = size;
	arrayStartLocation_ = arrayLocation;
	resources_.resize(size_);
}