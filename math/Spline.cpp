#include "Spline.h"
#include <limits>

using namespace OriGine;

Vec3f OriGine::CatmullRomSpline(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

std::deque<Vec3f> OriGine::CatmullRomSpline(const std::deque<Vec3f>& points, int samplePerSegment) {
    std::deque<Vec3f> result;

    /// コントロールポイントが4個以下なら何もできない
    if (points.size() < 4) {
        return result;
    }

    for (size_t i = 0; i < points.size() - 3; i++) {
        for (size_t s = 0; s < samplePerSegment; s++) {
            float t = static_cast<float>(s) / samplePerSegment;
            result.push_back(CatmullRomSpline(
                points[i + 0],
                points[i + 1],
                points[i + 2],
                points[i + 3],
                t));
        }
    }

    // 最後のポイントも追加（p2）
    result.push_back(points[points.size() - 2]);
    return result;
}

float OriGine::CalcSplineLength(const std::deque<Vec3f>& points) {
    if (points.empty()) {
        return 0.f;
    }
    float totalLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1 = points[i];
        Vec3f p2 = points[i + 1];
        totalLength += Vec3f(p2 - p1).length();
    }
    return totalLength;
}

std::pair<uint32_t, uint32_t> OriGine::CalcPointSegmentIndex(const std::deque<Vec3f>& points, const Vec3f& position) {
    if (points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1 = 0;
    uint32_t closestIndex2 = 1;
    float minDistanceSq    = (std::numeric_limits<float>::max)();
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1 = points[i];
        Vec3f p2 = points[i + 1];
        // 線分p1-p2に対する点positionの射影を計算
        Vec3f lineDir      = p2 - p1;
        float lineLengthSq = lineDir.lengthSq();
        if (lineLengthSq == 0.0f) {
            continue; // 同じ点の場合はスキップ
        }

        float t = (Vec3f(position - p1).dot(lineDir)) / lineLengthSq;
        t       = std::clamp(t, 0.0f, 1.0f);

        Vec3f projection = p1 + lineDir * t;
        float distanceSq = Vec3f(position - projection).lengthSq();
        // 最小距離を更新
        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
        }
    }
    return {closestIndex1, closestIndex2};
}

std::pair<uint32_t, uint32_t> OriGine::CalcDistanceSegmentIndex(const std::deque<Vec3f>& points, float distance) {
    if (points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1  = 0;
    uint32_t closestIndex2  = 1;
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        if (accumulatedLength + segmentLength >= distance) {
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
            break;
        }
        accumulatedLength += segmentLength;
    }
    return {closestIndex1, closestIndex2};
}

float OriGine::CalcDistanceAlongSpline(const std::deque<Vec3f>& points, const Vec3f& position) {
    if (points.empty()) {
        return 0.f;
    }
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        // 線分p1-p2に対する点positionの射影を計算
        Vec3f lineDir      = p2 - p1;
        float lineLengthSq = lineDir.lengthSq();
        if (lineLengthSq == 0.0f) {
            continue; // 同じ点の場合はスキップ
        }
        float t          = (Vec3f(position - p1).dot(lineDir)) / lineLengthSq;
        t                = std::clamp(t, 0.0f, 1.0f);
        Vec3f projection = p1 + lineDir * t;
        // 射影点が線分内にある場合、距離を計算して返す
        if (t >= 0.0f && t <= 1.0f) {
            return accumulatedLength + Vec3f(projection - p1).length();
        }
        accumulatedLength += segmentLength;
    }
    // 指定位置が全長を超える場合、全長を返す
    return accumulatedLength;
}

Vec3f OriGine::CalcPointOnSplineByDistance(const std::deque<Vec3f>& points, float distance) {
    if (points.empty()) {
        return Vec3f();
    }
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        if (accumulatedLength + segmentLength >= distance) {
            float remainingDistance = distance - accumulatedLength;
            float t                 = remainingDistance / segmentLength;
            // 線形補間で点を計算
            return p1 + (p2 - p1) * t;
        }
        accumulatedLength += segmentLength;
    }
    // 指定距離が全長を超える場合、最後の点を返す
    return points.back();
}
