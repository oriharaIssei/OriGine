#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "directX12/IConstantBuffer.h"

#include "Vector3.h"
#include "Vector4.h"

struct PointLight{
public:
	PointLight() = default;
	~PointLight(){}
	void DebugUpdate();

	Vector3 color   = {1.0f,1.0f,1.0f};
	Vector3 pos     = {0,0,0};
	float intensity = 0.1f;
	float radius    = 0.0f;
	float decay     = 0.1f;

public:
	struct ConstantBuffer{
		Vector3 color;      // 12 bytes
		float padding1;     // 4 bytes to align to 16 bytes
		Vector3 pos;        // 12 bytes
		float intensity;    // 4 bytes
		float radius;       // 4 bytes
		float decay;        // 4 bytes
		float padding2[2];  // 8 bytes (to align to 16 bytes)
		ConstantBuffer& operator=(const PointLight& light){
			color    = light.color;
			pos      = light.pos;
			intensity = light.intensity;
			radius    = light.radius;
			decay     = light.decay;
			return *this;
		}
	};
};