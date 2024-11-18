#pragma once

#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "material/Material.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

class Particle;
class Emitter{
public:
	void Init(const std::string& emitterName);
	void Update();
	void Draw(const IConstantBuffer<CameraTransform> camera);
private:
	std::string emitterName_;
	int32_t particleValue_;

	/// <summary>
	/// 頂点とMaterial を 併せ持つ
	/// </summary>
	Model* model_;
	IStructuredBuffer<Transform> structuredTransform_;

	float currentTime_;
	float spawnCoolTime_;
};