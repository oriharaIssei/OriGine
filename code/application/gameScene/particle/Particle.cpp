#include "Particle.h"

constexpr float delTime = 1.0f / 60.0f;

void Particle::Init(std::mt19937 &randomEngine){
	color = {1.0f,1.0f,1.0f,1.0f};

	std::uniform_real_distribution<float> distribution(-1.0f,1.0f);
	pos = {distribution(randomEngine),
		distribution(randomEngine),
		distribution(randomEngine)};

	velocity = {distribution(randomEngine),
		distribution(randomEngine),
		distribution(randomEngine)};

	std::uniform_real_distribution<float> lifeTimeDist(0.3f,1.5f);
	fullLifeTime_ = lifeTimeDist(randomEngine);
	lifeTime_ = fullLifeTime_;

	isAlive_ = true;
}

void Particle::Update(){
	if(!isAlive_){
		return;
	}
	lifeTime_ -= delTime;
	color.w = lifeTime_ / fullLifeTime_;

	pos += velocity * delTime;

	isAlive_ = lifeTime_ >= 0;
}
