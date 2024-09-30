#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Vector3.h"
#include "Vector4.h"

class SpotLight{
public:
	void Init();
	void Finalize();

	void DebugUpdate();

	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;
	void ConvertToBuffer();

	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	Vector3 pos = {0,0,0};
	float intensity;
	Vector3 direction = {0,0,-1.0f};
	float distance = 1.0f;
	float decay = 0.1f;
	float cosAngle = 0.5f;
	float cosFalloffStart = 1.0f;
private:
	struct ConstBuffer{
		Vector4 color;
		Vector3 pos;
		float intensity;
		Vector3 direction;
		float distance;// ライトが届く最大距離
		float decay;// 減衰率
		float cosAngle; //余弦
		float cosFalloffStart;
		float padding[2];
	};
private:
	ConstBuffer *mappingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
};

