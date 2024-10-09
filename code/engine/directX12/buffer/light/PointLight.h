#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "directX12/buffer/IConstantBuffer.h"

#include "Vector3.h"
#include "Vector4.h"

class PointLight
	:public IConstantBuffer{
public:
	PointLight() = default;
	~PointLight()override{}

	void Init()override;
	void Finalize()override;

	void DebugUpdate();

	void ConvertToBuffer()override;

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 pos = {0,0,0};
	float intensity;
	float radius;
	float decay = 0.1f;
	
private:
	struct ConstantBuffer{
	Vector3 color;      // 12 bytes
	float padding1;     // 4 bytes to align to 16 bytes
	Vector3 pos;        // 12 bytes
	float intensity;    // 4 bytes
	float radius;       // 4 bytes
	float decay;        // 4 bytes
	float padding2[2];  // 8 bytes (to align to 16 bytes)
	};
private:
	ConstantBuffer *mappingData_;
};