#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "directX12/buffer/IConstantBuffer.h"

#include "Vector3.h"
#include "Vector4.h"

class SpotLight
:public IConstantBuffer{
public:
	SpotLight() = default;
	~SpotLight()override{}

	void Init()    override;
	void Finalize()override;

	void DebugUpdate();

	void ConvertToBuffer()override;

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 pos = {0,0,0};
	float intensity = 0.1f;
	Vector3 direction = {0,0,-1.0f};
	float distance = 1.0f;
	float decay = 0.1f;
	float cosAngle = 0.5f;
	float cosFalloffStart = 1.0f;
private:
	struct ConstantBuffer{
		Vector3 color;              // 12 bytes
		Vector3 pos;                // 12 bytes
		float intensity;            // 4 bytes
		Vector3 direction;          // 12 bytes
		float distance;             // 4 bytes
		float decay;                // 4 bytes
		float cosAngle;             // 4 bytes
		float cosFalloffStart;      // 4 bytes
		float padding[2];           // 8 bytes (to align to 16 bytes)
	};
private:
	ConstantBuffer* mappingData_;
};

