#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "globalVariables/SerializedField.h"
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
	Emitter(DxSrvArray* srvArray,const std::string& emitterName);
	~Emitter();

	void Init();
	void Update(float deltaTime);

	void Draw();

#ifdef _DEBUG
	void Debug();
#endif // _DEBUG

	void CalculateMaxSize();
private:
	void SpawnParticle();
private:
	std::string emitterName_;
	DxSrvArray* srvArray_ = nullptr;

	std::vector<std::unique_ptr<Particle>> particles_;

	bool isActive_;
	SerializedField<bool> isLoop_;
	SerializedField<float> activeTime_;
	float leftActiveTime_;

	SerializedField<Vector3> originPos_;

	SerializedField<std::string> modelFileName_;
	SerializedField<std::string> textureFileName_;

	SerializedField<int32_t> shapeType_;
	std::unique_ptr<EmitterShape> emitterSpawnShape_;

	float currentCoolTime_ = 0.0f;
	SerializedField<float> spawnCoolTime_;
	SerializedField<float> particleLifeTime_;

	/// <summary>
	/// 一度に 生成される Particle の 数
	/// </summary>
	SerializedField<int32_t> spawnParticleVal_;
	/// <summary>
	/// 画面上に生成される 最大個数
	/// </summary>
	SerializedField<int32_t> particleMaxSize_;

	/// <summary>
	/// 頂点とMaterial を 併せ持つ
	/// </summary>
	std::unique_ptr<Model> particleModel_;
	IStructuredBuffer<ParticleTransform> structuredTransform_;

	/// <summary>
	/// パーティクルの 初期値
	/// </summary>
	SerializedField<Vector3> minDirectory_;
	SerializedField<Vector3> maxDirectory_;
	SerializedField<float> particleSpeed_;

	SerializedField<Vector3> particleScale_;
	SerializedField<Vector3> particleRotate_;

	SerializedField<Vector3> particleUvScale_;
	SerializedField<Vector3> particleUvRotate_;
	SerializedField<Vector3> particleUvTranslate_;

	SerializedField<Vector4> particleColor_;

	SerializedField<bool> particleIsBillBoard_;
public:
	bool getIsActive()const{ return isActive_; }
};
