#pragma once

#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "material/Material.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"
#include "transform/Transform.h"

#include "../Particle.h"

class Particle;
struct EmitterShape;
class Emitter{
public:
	Emitter(DxSrvArray* srvArray);
	~Emitter();

	void Init(const std::string& emitterName);
	void Update(float deltaTime);
	void Draw(const IConstantBuffer<CameraTransform>& camera);

#ifdef _DEBUG
	void Debug(bool* isOpenedWindow);
#endif // _DEBUG

private:
	void SpawnParticle();
private:
#ifdef _DEBUG
	int32_t shapeType_ = 0;
	std::string currentModelFileName_;
#endif // _DEBUG

private:
	DxSrvArray* srvArray_;

	std::string emitterName_;
	std::vector<std::unique_ptr<Particle>> particles_;

	std::unique_ptr<EmitterShape> emitterSpawnShape_;
	std::vector<int32_t> activeIndices_;

	float currentCoolTime_ 	= 0.0f;
	float spawnCoolTime_ 	= 0.1f;
	float particleLifeTime_ = 0.5f;

	/// <summary>
	/// 一度に 生成される Particle の 数
	/// </summary>
	int32_t spawnParticleVal_ = 1;

	int32_t particleMaxSize_ = 10;

	/// <summary>
	/// 頂点とMaterial を 併せ持つ
	/// </summary>
	std::unique_ptr<Model> particleModel_;
	IStructuredBuffer<ParticleTransform> structuredTransform_;

	/// <summary>
	/// パーティクルの 初期値
	/// </summary>
	ParticleTransform particleInitialTransform_;
};