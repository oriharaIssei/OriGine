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

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 pos = {0,0,0};
	float intensity;
	float radius;
	float decay = 0.1f;
	
private:
	struct ConstBuffer{
	Vector3 color;      // 12 bytes
	float padding1;     // 4 bytes to align to 16 bytes
	Vector3 pos;        // 12 bytes
	float intensity;    // 4 bytes
	float radius;       // 4 bytes
	float decay;        // 4 bytes
	float padding2[2];  // 8 bytes (to align to 16 bytes)
	};
private:
	ConstBuffer *mappingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
};