#pragma once

#include <stdint.h>

#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"

struct Model;
struct Transform;
struct ParticleTransform;
struct Vector3;

class Particle{
public:
	Particle() = default;
	~Particle(){}

	void Init(Model* _model,ParticleTransform* _transform,float _lifeTime);
	void Update(float deltaTime);

	void Spawn(const ParticleTransform& initialValue,float _lifeTime);
private:
	bool isAlive_;
	// 形状,Material
	Model* model_;
	// 位置，サイズ
	ParticleTransform* transform_;

	float lifeTime_;
public:
	bool getIsAlive()const{ return isAlive_; }
};