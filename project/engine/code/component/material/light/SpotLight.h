#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "globalVariables/SerializedField.h"

#include "Vector3.h"
#include "Vector4.h"

struct SpotLight{
public:
	SpotLight() = default;
	
	SpotLight(const std::string& scene,int32_t index):
		color{scene,"SpotLight" + std::to_string(index),"color"},
		pos{scene,"SpotLight" + std::to_string(index),"pos"},
		intensity{scene,"SpotLight" + std::to_string(index),"intensity"},
		direction{scene,"SpotLight" + std::to_string(index),"direction"},
		distance{scene,"SpotLight" + std::to_string(index),"distance"},
		decay{scene,"SpotLight" + std::to_string(index),"decay"},
		cosAngle{scene,"SpotLight" + std::to_string(index),"cosAngle"},
		cosFalloffStart{scene,"SpotLight" + std::to_string(index),"cosFalloffStart"}{}
	
	~SpotLight(){}

	SerializedField<Vec3f>color;
	SerializedField<Vec3f>pos;
	SerializedField<float>intensity;
	SerializedField<Vec3f>direction;
	SerializedField<float>distance;
	SerializedField<float>decay;
	SerializedField<float>cosAngle;
	SerializedField<float> cosFalloffStart;
public:
	struct ConstantBuffer{
		Vec3f color;              // 12 bytes
		float intensity;            // 4 bytes
		Vec3f pos;                // 12 bytes
		float distance;             // 4 bytes
		Vec3f direction;          // 12 bytes
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

