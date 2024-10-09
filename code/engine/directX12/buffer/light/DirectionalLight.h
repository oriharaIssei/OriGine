#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "directX12/buffer/IConstantBuffer.h"

#include "Vector3.h"
#include "Vector4.h"

class DirectionalLight
 : public IConstantBuffer{
public:
	void Init()    override;
	void Finalize()override;

	void DebugUpdate();

	void ConvertToBuffer()override;

	Vector3 color = {1.0f,1.0f,1.0f};
	Vector3 direction = {1.0f,1.0f,1.0f};
	float intensity = 1.0f;
private:
	struct ConstantBuffer{
		Vector3 color;      // 12 bytes
		float padding1;     // 4 bytes to align to 16 bytes
		Vector3 direction;  // 12 bytes
		float intensity;    // 4 bytes
	};

private:
	ConstantBuffer *mappingData_;
};