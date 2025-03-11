#pragma once

/// stl
#include <concepts>

/// math
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"

class BinaryWriter;
class BinaryReader;

class IShape {
public:
    IShape() {}
    virtual ~IShape() = 0;

    virtual bool Edit() = 0;

    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;
};

/// <summary>
/// IShapeを継承しているか
/// </summary>
template <typename T>
concept IsShape = std::derived_from<T, IShape>;

class Sphere
    : public IShape {
public:
    Sphere() {}
    Sphere(const Vec3f& _center, float _radius)
        : center_(_center), radius_(_radius) {}
    ~Sphere() {}

    bool Edit() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    Vec3f center_ = {};
    float radius_ = 0.f;
};

class AABB
    : public IShape {
public:
    AABB() {}
    AABB(const Vec3f& _min, const Vec3f& _max)
        : min_(_min), max_(_max) {}
    ~AABB() {}

    bool Edit() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    Vec3f min_ = {};
    Vec3f max_ = {};
};

class OBB
    : public AABB {
public:
    OBB() {}
    OBB(const Vec3f& _min, const Vec3f& _max, const Quaternion& _rotate)
        : AABB(_min, _max), rotate_(_rotate) {}
    ~OBB() {}

    bool Edit() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    Quaternion rotate_ = {};
};

class Capsule
    : public IShape {
public:
    Capsule() {}
    Capsule(const Vec3f& _start, const Vec3f& _end, float _radius)
        : start_(_start), end_(_end), radius_(_radius) {}
    ~Capsule() {}

    bool Edit() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    Vec3f start_  = {};
    Vec3f end_    = {};
    float radius_ = 0.f;
};
