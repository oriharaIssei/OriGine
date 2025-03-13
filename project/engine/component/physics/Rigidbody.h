#pragma once

/// parent
#include "component/IComponent.h"

/// math
#include "Vector3.h"

class Rigidbody
    : public IComponent {
public:
    Rigidbody();
    virtual ~Rigidbody() = default;

    void Init(GameEntity* _entity) override;

    virtual bool Edit();
    virtual void Save(BinaryWriter& _writer);
    virtual void Load(BinaryReader& _reader);

    virtual void Finalize();

private:
    Vec3f acceleration_ = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f velocity_     = Vec3f(0.0f, 0.0f, 0.0f);

    float mass_      = 1.0f;
    bool useGravity_ = false;

public: // accsessor
    const Vec3f& getAcceleration() const { return acceleration_; }
    void setAcceleration(const Vec3f& _acceleration) { acceleration_ = _acceleration; }

    const Vec3f& getVelocity() const { return velocity_; }
    void setVelocity(const Vec3f& _velocity) { velocity_ = _velocity; }

    float getMass() const { return mass_; }
    void setMass(const float _mass) { mass_ = _mass; }

    bool getUseGravity() const { return useGravity_; }
    void setUseGravity(const bool _isGravity) { useGravity_ = _isGravity; }
};
