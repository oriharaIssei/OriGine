#include "Spline.h"
#include <limits>

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

float OriGine::CalcSplineLength(const std::deque<Vec3f>& _points) {
    if (_points.empty()) {
        return 0.f;
    }
    float totalLength = 0.0f;
    for (size_t i = 0; i < _points.size() - 1; i++) {
        Vec3f p1 = _points[i];
        Vec3f p2 = _points[i + 1];
        totalLength += Vec3f(p2 - p1).length();
    }
    return totalLength;
}

std::pair<uint32_t, uint32_t> OriGine::CalcPointSegmentIndex(const std::deque<Vec3f>& _points, const Vec3f& _position) {
    if (_points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1 = 0;
    uint32_t closestIndex2 = 1;
    float minDistanceSq    = (std::numeric_limits<float>::max)();
    for (size_t i = 0; i < _points.size() - 1; i++) {
        Vec3f p1 = _points[i];
        Vec3f p2 = _points[i + 1];
        // 線分p1-p2に対する点_positionの射影を計算
        Vec3f lineDir      = p2 - p1;
        float lineLengthSq = lineDir.lengthSq();
        if (lineLengthSq == 0.0f) {
            continue; // 同じ点の場合はスキップ
        }

        float t = (Vec3f(_position - p1).dot(lineDir)) / lineLengthSq;
        t       = std::clamp(t, 0.0f, 1.0f);

        Vec3f projection = p1 + lineDir * t;
        float distanceSq = Vec3f(_position - projection).lengthSq();
        // 最小距離を更新
        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
        }
    }
    return {closestIndex1, closestIndex2};
}

std::pair<uint32_t, uint32_t> OriGine::CalcDistanceSegmentIndex(const std::deque<Vec3f>& _points, float _distance) {
    if (_points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1  = 0;
    uint32_t closestIndex2  = 1;
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < _points.size() - 1; i++) {
        Vec3f p1            = _points[i];
        Vec3f p2            = _points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        if (accumulatedLength + segmentLength >= _distance) {
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
            break;
        }
        accumulatedLength += segmentLength;
    }
    return {closestIndex1, closestIndex2};
}

Vec3f OriGine::CalcPointOnSplineByDistance(const std::deque<Vec3f>& _points, float _distance) {
    if (_points.empty()) {
        return Vec3f();
    }
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < _points.size() - 1; i++) {
        Vec3f p1            = _points[i];
        Vec3f p2            = _points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        if (accumulatedLength + segmentLength >= _distance) {
            float remainingDistance = _distance - accumulatedLength;
            float t                 = remainingDistance / segmentLength;
            // 線形補間で点を計算
            return p1 + (p2 - p1) * t;
        }
        accumulatedLength += segmentLength;
    }
    // 指定距離が全長を超える場合、最後の点を返す
    return _points.back();
}
