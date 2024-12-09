#pragma once

#include <memory>
#include <stdint.h>

#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"

struct Transform;
struct Vector3;

class Particle{
public:
	Particle();
	virtual ~Particle();

	void Init(const ParticleTransform& transform,
			  Vector3 velocity,
			  float lifeTime);
	void Update(float deltaTime);
protected:
	ParticleTransform transform_;

	Vector3 velocity_;
	float maxLifeTime_;
	float lifeTime_;
	bool isAlive_;
public:
	const ParticleTransform& getTransform()const{ return transform_; }
	bool getIsAlive()const{ return isAlive_; }
};