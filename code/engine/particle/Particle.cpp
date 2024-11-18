#include "Particle.h"


void Particle::Init(Model* _model,uint32_t _textureIndex,Transform* _transform,float _lifeTime){
	model_ = _model;
	textureIndex_ = _textureIndex;
	transform_ = _transform;
	lifeTime_ = _lifeTime;
}

void Particle::Draw(IConstantBuffer<CameraTransform> camera){

}