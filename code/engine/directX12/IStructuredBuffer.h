#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

#include "directX12/DxHeap.h"
#include "directX12/DxResource.h"
#include "directX12/DxSrvArray.h"

template<typename T>
concept HasInConstantBuffer = requires {
	typename T::ConstantBuffer;
	requires std::is_copy_assignable_v<typename T::ConstantBuffer>;
	{ std::declval<typename T::ConstantBuffer>() = std::declval<const T&>() } -> std::same_as<typename T::ConstantBuffer&>;
};

template<HasInConstantBuffer structBuff>
class IStructuredBuffer{
public:
	IStructuredBuffer() = default;
	~IStructuredBuffer(){ Finalize(); }

	void CreateBuffer(ID3D12Device* device,DxSrvArray* srvArray,uint32_t elementCount);
	void Finalize(){ buff_.Finalize(); }

	void ConvertToBuffer();
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const;

	// 公開用変数（バッファのデータを保持）
	std::vector<typename structBuff> openData_;
protected:
	DxSrvArray* srvArray_;
	int32_t srvIndex_;
	DxResource buff_;

	// bind されたデータへのポインタ
	typename structBuff::ConstantBuffer* mappingData_ = nullptr;

	uint32_t elementCount_ = 0;
public:
	const DxResource& getResource(){ return buff_; }
};

template<HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::CreateBuffer(ID3D12Device* device,DxSrvArray* srvArray,uint32_t elementCount){
	if(elementCount_ == 0){
		return;
	}
	elementCount_ = elementCount;
	size_t bufferSize = sizeof(typename structBuff::ConstantBuffer) * elementCount_;
	buff_.CreateBufferResource(device,bufferSize);
	buff_.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&mappingData_));

	openData_.resize(elementCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	lightViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	lightViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	lightViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	lightViewDesc.Buffer.FirstElement = 0;
	lightViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	lightViewDesc.Buffer.NumElements = elementCount;
	lightViewDesc.Buffer.StructureByteStride = sizeof(structBuff::ConstantBuffer);

	srvIndex_ = srvArray_->CreateView(device,viewDesc,buff_.getResource());
}

template<HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::ConvertToBuffer(){
	if(mappingData_ && openData_.size() == elementCount_){
		std::copy(openData_.begin(),openData_.end(),mappingData_);
	}
}

template<HasInConstantBuffer structBuff>
inline void IStructuredBuffer<structBuff>::SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const{
	cmdList->SetGraphicsRootDescriptorTable(rootParameterNum,DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(srvIndex_)));
}