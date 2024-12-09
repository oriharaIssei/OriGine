#include "Particle.h"

#include "transform/ParticleTransform.h"

Particle::Particle(){}

Particle::~Particle(){}

void Particle::Init(const ParticleTransform& transform,
					Vector3 velocity,
					float lifeTime){
	transform_ = transform;
	transform_.UpdateMatrix();

	velocity_ = velocity;

	isAlive_ = true;
	maxLifeTime_ = lifeTime;
	lifeTime_ = maxLifeTime_;
}

void Particle::Update(float deltaTime){
	if(!isAlive_){
		return;
	}

	lifeTime_ -= deltaTime;
	if(lifeTime_ <= 0.0f){
		isAlive_ = false;
		return;
	}

	transform_.translate += velocity_ * deltaTime;

	transform_.UpdateMatrix();
}