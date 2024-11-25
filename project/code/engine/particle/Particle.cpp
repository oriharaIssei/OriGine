#include "Particle.h"

#include "transform/ParticleTransform.h"

void Particle::Init(Model* _model,ParticleTransform* _transform,float _lifeTime){
	model_ = _model;
	transform_ = _transform;
	lifeTime_ = _lifeTime;
}

void Particle::Update(float deltaTime){
	if(!isAlive_){
		transform_->color.w = 0.0f;
		return;
	}

}

void Particle::Spawn(const ParticleTransform& initialValue,float _lifeTime){
	lifeTime_ = _lifeTime;

	*transform_ = initialValue;
}
