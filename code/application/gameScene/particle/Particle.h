#pragma once

#include <random>

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

struct ParticleStructuredBuffer{
	Matrix4x4 transform;
	Vector4 color;

	struct ConstantBuffer{
		Matrix4x4 transform;
		Vector4 color;
		ConstantBuffer& operator=(const ParticleStructuredBuffer& buff){
			transform = buff.transform;
			color = buff.color;
			return *this;
		}
	};
};

struct Particle{
	void Init(std::mt19937& randomEngine);
	void Update();
	Vector3 pos;
	Vector4 color;
	Vector3 velocity;

	float lifeTime_;
	float fullLifeTime_;

	bool isAlive_;
};