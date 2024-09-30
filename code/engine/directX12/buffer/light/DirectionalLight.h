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

	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	Vector3 direction = {1.0f,1.0f,1.0f};
	float intensity = 1.0f;
private:
	struct ConstBuffer{
		Vector4 color;
		Vector3 direction;// ライトの向き
		float intensity;// 輝度
	};
private:
	ConstBuffer *mappingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
};