#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "Vector3.h"
#include "Vector4.h"

struct SpotLight{
public:
	SpotLight() = default;
	~SpotLight(){}

	void Init(const std::string& scene,int32_t index);

	Vector3 color         = {1.0f,1.0f,1.0f};
	Vector3 pos           = {0,0,0};
	float intensity       = 0.1f;
	Vector3 direction     = {0,0,-1.0f};
	float distance        = 1.0f;
	float decay           = 0.1f;
	float cosAngle        = 0.5f;
	float cosFalloffStart = 1.0f;
public:
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
		ConstantBuffer& operator=(const SpotLight& light){
			color           = light.color;
			pos             = light.pos;
			intensity       = light.intensity;
			direction       = light.direction;
			distance        = light.distance;
			decay           = light.decay;
			cosAngle        = light.cosAngle;
			cosFalloffStart = light.cosFalloffStart;
			return *this;
		}
	};
};

