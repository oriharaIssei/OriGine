#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Vector3.h"
#include "Vector4.h"

struct DirectionalLight{
public:
	void Init(int32_t num);
	void DebugUpdate();

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 direction = {1.0f,1.0f,1.0f};
	float intensity = 1.0f;
public:
	struct ConstantBuffer{
		Vector3 color;      // 12 バイト
		float intensity;    // 4 バイト（合計16バイト）
		Vector3 direction;  // 12 バイト
		float padding;      // 4 バイト（合計16バイト）
		ConstantBuffer& operator=(const DirectionalLight& light){
			color     = light.color;
			direction = light.direction;
			intensity = light.intensity;
			return *this;
		}
	};
};