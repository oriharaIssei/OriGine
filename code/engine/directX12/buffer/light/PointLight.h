#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Vector3.h"
#include "Vector4.h"

class PointLight{
public:
	void Init();
	void Finalize();

	void DebugUpdate();

	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;
	void ConvertToBuffer();

	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	Vector3 pos = {0,0,0};
	float intensity;
	float radius;
	float decay;
	
private:
	struct ConstBuffer{
		Vector4 color;
		Vector3 pos;
		float intensity;
		float radius;
		float decay;
		float padding[2];
	};
private:
	ConstBuffer *mappingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
};