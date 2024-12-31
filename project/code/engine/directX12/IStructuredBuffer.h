#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

#include "directX12/DxHeap.h"
#include "directX12/DxResource.h"
#include "directX12/DxSrvArray.h"

template<typename T>
concept StructuredBuffer = requires {
	typename T::ConstantBuffer;
	requires std::is_copy_assignable_v<typename T::ConstantBuffer>;
	{ std::declval<typename T::ConstantBuffer>() = std::declval<const T&>() } -> std::same_as<typename T::ConstantBuffer&>;
};
template<StructuredBuffer structBuff>
class IStructuredBuffer{
public:
	IStructuredBuffer() = default;
	~IStructuredBuffer(){ Finalize(); }

	void CreateBuffer(ID3D12Device* device,DxSrvArray* srvArray,uint32_t elementCount);
	void Finalize(){
		if(srvArray_){
			srvArray_->DestroyView(srvIndex_);
		}
		buff_.Finalize();
	}

	void ConvertToBuffer();
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const;

	void Resize(ID3D12Device* device,uint32_t newElementCount);
	void ResizeForDataSize(ID3D12Device* device);

	// 公開用変数（バッファのデータを保持）
	std::vector<structBuff> openData_;
protected:
	DxSrvArray* srvArray_ = nullptr;
	int32_t srvIndex_ = 0;
	DxResource buff_;

	// bind されたデータへのポインタ
	structBuff::ConstantBuffer* mappingData_ = nullptr;

	uint32_t elementCount_ = 0;
public:
	const DxResource& getResource(){ return buff_; }
};

template<StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::CreateBuffer(ID3D12Device* device,DxSrvArray* srvArray,uint32_t elementCount){
	elementCount_ = elementCount;
	srvArray_ = srvArray;

	if(elementCount_ == 0){
		return;
	}

	size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
	buff_.CreateBufferResource(device,bufferSize);
	buff_.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&mappingData_));

	openData_.reserve(elementCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	viewDesc.Buffer.NumElements = elementCount;
	viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

	srvIndex_ = srvArray_->CreateView(device,viewDesc,buff_.getResource());
}

template<StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::Resize(ID3D12Device* device,uint32_t newElementCount){
	if(newElementCount == elementCount_ || newElementCount == 0){
		return;
	}

	// 今のバッファを削除
	Finalize();

	{ // 新しいバッファを作成
		elementCount_ = newElementCount;

		size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
		buff_.CreateBufferResource(device,bufferSize);
		buff_.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&mappingData_));

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		viewDesc.Buffer.NumElements = elementCount_;
		viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

		srvIndex_ = srvArray_->CreateView(device,viewDesc,buff_.getResource());
	}
}

template<StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ResizeForDataSize(ID3D12Device* device){
	int32_t newElementCount = static_cast<int32_t>(openData_.size());
	if(newElementCount == elementCount_ || newElementCount == 0){
		return;
	}

	// 今のバッファを削除
	Finalize();

	{ // 新しいバッファを作成
		elementCount_ = newElementCount;

		size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
		buff_.CreateBufferResource(device,bufferSize);
		buff_.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&mappingData_));

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		viewDesc.Buffer.NumElements = elementCount_;
		viewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

		srvIndex_ = srvArray_->CreateView(device,viewDesc,buff_.getResource());
	}
}

template<StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ConvertToBuffer(){
	if(mappingData_){
		std::copy(openData_.begin(),openData_.end(),mappingData_);
	}
}

template<StructuredBuffer structBuff>
inline void IStructuredBuffer<structBuff>::SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const{
	cmdList->SetGraphicsRootDescriptorTable(rootParameterNum,DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(srvIndex_)));
}
