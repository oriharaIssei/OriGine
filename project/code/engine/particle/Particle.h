#pragma once

#include <stdint.h>

#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"

struct Transform;
struct Vector3;

class Particle{
public:
	Particle() = default;
	~Particle(){}

	void Init(const ParticleTransform& transform,float lifeTime);
	void Update(float deltaTime);
private:
	ParticleTransform transform_;

	float lifeTime_;
	bool isAlive_;
public:
	const ParticleTransform& getTransform()const{ return transform_; }
	bool getIsAlive()const{ return isAlive_; }
};