#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "Vector3.h"
#include "Vector4.h"

class DirectionalLight{
public:
	void Init();
	void Finalize();

	void DebugUpdate();

	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;
	void ConvertToBuffer();

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 direction = {1.0f,1.0f,1.0f};
	float intensity = 1.0f;
private:
	struct ConstBuffer{
		Vector3 color;      // 12 bytes
		float padding1;     // 4 bytes to align to 16 bytes
		Vector3 direction;  // 12 bytes
		float intensity;    // 4 bytes
	};

private:
	ConstBuffer *mappingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
};