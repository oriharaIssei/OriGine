#pragma once

#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "material/Material.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"
#include "transform/Transform.h"

#include "../Particle.h"

class Emitter{
public:
	void Init(const std::string& emitterName);
	void Update(float deltaTime);
	void Draw(const IConstantBuffer<CameraTransform>& camera);

#ifdef _DEBUG
	void Debug();
#endif // _DEBUG

private:
	void SpawnParticle();
private:
	std::string emitterName_;
	std::list<std::unique_ptr<Particle>> particles_;

	float currentCoolTime_;
	float spawnCoolTime_;
	/// <summary>
	/// 一度に 生成される Particle の 数
	/// </summary>
	int32_t spawnParticleVal_;

	int32_t particleMaxSize_;

	/// <summary>
	/// 頂点とMaterial を 併せ持つ
	/// </summary>
	Model* particleModel_;
	IStructuredBuffer<ParticleTransform> structuredTransform_;

	/// <summary>
	/// パーティクルの 初期値
	/// </summary>
	ParticleTransform particleInitialTransform_;

	float particleLifeTime_;
};