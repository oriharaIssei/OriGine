#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "globalVariables/SerializedField.h"

#include "Vector3.h"
#include "Vector4.h"

struct DirectionalLight{
public:
	DirectionalLight(const std::string& scene,int32_t index):
		color{scene,"DirectionalLight" + std::to_string(index),"color"},
		direction{scene,"DirectionalLight" + std::to_string(index),"direction"},
		intensity{scene,"DirectionalLight" + std::to_string(index),"intensity"}{}

	~DirectionalLight(){}

	SerializedField<Vector3>color;
	SerializedField<Vector3>direction;
	SerializedField<float>intensity;
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