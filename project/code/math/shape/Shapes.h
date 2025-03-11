#pragma once

/// stl
#include <concepts>

/// math
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct IShape {
    virtual ~IShape() {}
};

template <typename T>
concept IsShape = std::derived_from<T, IShape>;

struct AABB
    : public IShape {
    AABB() {}
    AABB(const Vec3f& _min, const Vec3f& _max) : min_(_min), max_(_max) {}

    Vec3f min_ = {0.f, 0.f, 0.f};
    Vec3f max_ = {0.f, 0.f, 0.f};
};

struct Sphere
    : public IShape {
    Sphere(){}
    Sphere(const Vec3f& _center, float _radius) : center_(_center), radius_(_radius) {}

    Vec3f center_ = {0.f, 0.f, 0.f};
    float radius_ = 0.f;
};
