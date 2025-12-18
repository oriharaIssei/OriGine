#pragma once

/// stl
#include <deque>

/// math
#include "Vector3.h"

namespace OriGine {

Vec3f CatmullRomSpline(const Vec3f& _p0, const Vec3f& _p1, const Vec3f& _p2, const Vec3f& _p3, float _t);

std::deque<Vec3f> CatmullRomSpline(const std::deque<Vec3f>& _points, int _samplePerSegment);
} // namespace OriGine
