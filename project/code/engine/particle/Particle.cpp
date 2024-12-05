#include "Particle.h"

#include "transform/ParticleTransform.h"

void Particle::Init(const ParticleTransform& transform,float lifeTime){
	transform_ = transform;

	isAlive_ = true;
	lifeTime_ = lifeTime;
}

void Particle::Update(float deltaTime){
	if(!isAlive_){
		return;
	}

	lifeTime_ -= deltaTime;
	if(lifeTime_ <= 0.0f){
		isAlive_ = false;
	}
}