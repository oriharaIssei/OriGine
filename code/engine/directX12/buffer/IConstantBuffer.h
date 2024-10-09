#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <type_traits>
#include <wrl.h>

class IConstantBuffer{
public:
	IConstantBuffer() = default;
	virtual ~IConstantBuffer(){}

	virtual void Init  () = 0;
	virtual void Update() = 0;
	virtual void Finalize() = 0;

	virtual void ConvertToBuffer() = 0;
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const;
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> buff_;
};

template<typename T>
concept HasInConstantBuffer = requires {
	requires T::ConstantBuffer;
	requires std::is_base_of_v<IConstantBuffer,T>;
};
