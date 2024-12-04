#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "globalVariables/SerializedField.h"

#include "Vector3.h"
#include "Vector4.h"

struct PointLight{
public:
	PointLight(const std::string& scene,int32_t index):
		color{scene,"PointLight" + std::to_string(index),"color"},
		pos{scene,"PointLight" + std::to_string(index),"pos"},
		intensity{scene,"PointLight" + std::to_string(index),"intensity"},
		radius{scene,"PointLight" + std::to_string(index),"radius"},
		decay{scene,"PointLight" + std::to_string(index),"decay"}{}

	~PointLight(){}

	SerializedField<Vector3> color;
	SerializedField<Vector3> pos;
	SerializedField<float> intensity;
	SerializedField<float> radius;
	SerializedField<float> decay;

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