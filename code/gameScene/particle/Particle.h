#pragma once

#include <random>

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

struct ParticleStructuredBuffer{
	Matrix4x4 Transform;
	Vector4 color;
};

struct Particle{
	void Init(std::mt19937 &randomEngine);
	void Update();
	Vector3 pos;
	Vector4 color;
	Vector3 velocity;

	float lifeTime_;
	float fullLifeTime_;

	bool isAlive_;
};