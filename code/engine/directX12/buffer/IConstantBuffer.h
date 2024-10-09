#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>

class ID3D12GraphicsCommandList;


template<typename T>
concept HasInConstantBuffer = requires {
	typename T::ConstantBuffer;
	requires std::is_base_of_v<IConstantBuffer,typename T>;
};
class IConstantBuffer{
public:
	IConstantBuffer() = default;
	virtual ~IConstantBuffer() = 0;

	virtual void Init  () = 0;
	virtual void Update() = 0;

	virtual void ConvertToBuffer() = 0;
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const;
};