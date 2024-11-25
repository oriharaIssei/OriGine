#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>

#include "directX12/DxResource.h"

template<typename T>
concept HasInConstantBuffer = requires {
	typename T::ConstantBuffer;
	requires std::is_copy_assignable_v<typename T::ConstantBuffer>;
	{ std::declval<typename T::ConstantBuffer>() = std::declval<const T&>() } -> std::same_as<typename T::ConstantBuffer&>;
};

template<HasInConstantBuffer constBuff>
class IConstantBuffer{
public:
	IConstantBuffer() = default;
	~IConstantBuffer(){}

	void CreateBuffer(ID3D12Device* device);
	void Finalize(){ buff_.Finalize(); }

	void ConvertToBuffer();
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const;
	// 公開用変数
	constBuff openData_;
protected:
	// bind されたデータ
	constBuff::ConstantBuffer* mappingData_;
	DxResource buff_;
public:
	const DxResource& getResource(){ return buff_; }
};

template<HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::CreateBuffer(ID3D12Device* device){
	buff_.CreateBufferResource(device,sizeof(constBuff::ConstantBuffer));
	buff_.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&mappingData_));
}

template<HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::ConvertToBuffer(){
	*mappingData_ = openData_;
}

template<HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const{
	cmdList->SetGraphicsRootConstantBufferView(rootParameterNum,buff_.getResource()->GetGPUVirtualAddress());
}

