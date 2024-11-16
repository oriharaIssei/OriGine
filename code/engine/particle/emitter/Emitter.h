#pragma once

#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

class Particle;
class Emitter{
public:
	void Init();
	void Update();
	void Draw(const IConstantBuffer<CameraTransform> camera);
private:
	IStructuredBuffer<Transform> structuredTransform_;
};