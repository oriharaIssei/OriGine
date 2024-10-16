#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "Vector3.h"

Vector3 CatmullRomInterpolation(const Vector3& p0,const Vector3& p1,const Vector3& p2,const Vector3& p3,float t);

Vector3 CatmullRomInterpolation(const std::vector<Vector3>& points,float t);