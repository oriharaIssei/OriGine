#include "Spline.h"

using namespace OriGine;

Vec3f OriGine::CatmullRomSpline(const Vec3f& _p0, const Vec3f& _p1, const Vec3f& _p2, const Vec3f& _p3, float _t) {
    float t2 = _t * _t;
    float t3 = t2 * _t;

    return 0.5f * ((2.0f * _p1) + (-_p0 + _p2) * _t + (2.0f * _p0 - 5.0f * _p1 + 4.0f * _p2 - _p3) * t2 + (-_p0 + 3.0f * _p1 - 3.0f * _p2 + _p3) * t3);
}

std::deque<Vec3f> OriGine::CatmullRomSpline(const std::deque<Vec3f>& _points, int _samplePerSegment) {
    std::deque<Vec3f> result;

    /// コントロールポイントが4個以下なら何もできない
    if (_points.size() < 4) {
        return result;
    }

    for (size_t i = 0; i < _points.size() - 3; i++) {
        for (size_t s = 0; s < _samplePerSegment; s++) {
            float t = static_cast<float>(s) / _samplePerSegment;
            result.push_back(CatmullRomSpline(
                _points[i + 0],
                _points[i + 1],
                _points[i + 2],
                _points[i + 3],
                t));
        }
    }

    // 最後のポイントも追加（p2）
    result.push_back(_points[_points.size() - 2]);
    return result;
}
