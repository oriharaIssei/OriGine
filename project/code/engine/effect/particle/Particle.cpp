#include "Particle.h"

//transform
#include "transform/ParticleTransform.h"

Particle::Particle() {}

Particle::~Particle() {}

void Particle::Init(const ParticleTransform& _initialTransfrom, float _lifeTime) {
    transform_ = _initialTransfrom;
    transform_.UpdateMatrix();

    isAlive_ = true;

    lifeTime_ = _lifeTime;
    leftTime_ = lifeTime_;
}

void Particle::Update(float deltaTime) {
    if (!isAlive_) {
        return;
    }

    leftTime_ -= deltaTime;
    if (leftTime_ <= 0.0f) {
        isAlive_ = false;
        return;
    }

    transform_.translate += velocity_ * deltaTime;

    transform_.UpdateMatrix();
}
