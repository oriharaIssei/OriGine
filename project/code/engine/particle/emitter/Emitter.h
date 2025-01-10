#pragma once

///stl
#include <memory>
#include <vector>

///engine
//dxObject
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"

//lib
#include "globalVariables/SerializedField.h"

//asset
#include "material/Material.h"
//objct
#include "object3d/Object3d.h"
//transform
#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"
#include "transform/Transform.h"

// emitObject
#include "../Particle.h"
class Particle;
struct EmitterShape;
class Emitter{
public:
	Emitter(DxSrvArray* srvArray,const std::string& emitterName);
	~Emitter();

	void Init();
	void Update(float deltaTime);

	void Draw(const IConstantBuffer<CameraTransform>& camera);

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

	SerializedField<Vec3f> originPos_;

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
	SerializedField<Vec3f> minDirectory_;
	SerializedField<Vec3f> maxDirectory_;
	SerializedField<float> particleSpeed_;

	SerializedField<Vec3f> particleScale_;
	SerializedField<Vec3f> particleRotate_;

	SerializedField<Vec3f> particleUvScale_;
	SerializedField<Vec3f> particleUvRotate_;
	SerializedField<Vec3f> particleUvTranslate_;

	SerializedField<Vec4f> particleColor_;

	SerializedField<bool> particleIsBillBoard_;
public:
	bool getIsActive()const{ return isActive_; }
};
