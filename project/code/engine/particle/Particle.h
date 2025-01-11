#pragma once

///stl
#include <memory>

//transform
#include "transform/ParticleTransform.h"

///math
#include "Vector3.h"

#include <stdint.h>

class Particle {
public:
    Particle();
    virtual ~Particle();

    void Init(const ParticleTransform& transform, Vec3f velocity, float lifeTime);
    void Update(float deltaTime);

protected:
    ParticleTransform transform_;

    Vec3f velocity_;
    float maxLifeTime_;
    float lifeTime_;
    bool isAlive_;

public:
    const ParticleTransform& getTransform() const { return transform_; }
    bool getIsAlive() const { return isAlive_; }
};
