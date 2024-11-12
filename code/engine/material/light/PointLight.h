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

	void Init(int32_t num);
	void DebugUpdate();

	Vector3 color   = {1.0f,1.0f,1.0f};
	Vector3 pos     = {0,0,0};
	float intensity = 0.1f;
	float radius    = 0.0f;
	float decay     = 0.1f;

public:
	struct ConstantBuffer{
		Vector3 color;      // 12 バイト
		float intensity;    // 4 バイト（合計16バイトにアラインメント）

		Vector3 pos;        // 12 バイト
		float radius;       // 4 バイト（合計16バイトにアラインメント）

		float decay;        // 4 バイト
		float padding[3];   // 12 バイト（合計16バイトにアラインメント）
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