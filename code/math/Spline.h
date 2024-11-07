#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "Vector3.h"

Vector3 CatmullRomInterpolation(const Vector3& p0,const Vector3& p1,const Vector3& p2,const Vector3& p3,float t);

Vector3 CatmullRomInterpolation(const std::vector<Vector3>& points,float t);

struct ArcLengthSegment{
    float t;
    float length;
};

class Spline{
public:
    Spline(const std::vector<Vector3>& points);

    void CalculateArcLength();
private:
    const std::vector<Vector3>& controlPoints_;
    std::vector<ArcLengthSegment> arcLengthSegments_;
    float totalLength_ = 0.0f;
public:
    // tに対する位置を取得
    Vector3 GetPosition(float t) const{
        return CatmullRomInterpolation(controlPoints_,t);
    }

    // 距離に基づいてtを取得
    float GetTFromDistance(float distance)const;

    float GetTotalLength() const{ return totalLength_; }
};
