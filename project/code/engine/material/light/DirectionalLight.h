#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "Vector3.h"
#include "Vector4.h"

struct DirectionalLight{
public:
	void Init(const std::string& scene,int32_t index);

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 direction = {1.0f,1.0f,1.0f};
	float intensity = 1.0f;
public:
	struct ConstantBuffer{
		Vector3 color;      // 12 bytes
		float padding1;     // 4 bytes to align to 16 bytes
		Vector3 direction;  // 12 bytes
		float intensity;    // 4 bytes
		ConstantBuffer& operator=(const DirectionalLight& light){
			color     = light.color;
			direction = light.direction;
			intensity = light.intensity;
			return *this;
		}
	};
};