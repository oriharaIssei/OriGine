#pragma once

/// <summary>
/// コライダー形状からラインメッシュを生成するビルダー
/// </summary>

/// math
#include "math/bounds/base/IBounds.h"
#include <Matrix4x4.h>
#include <numbers>
#include <Vector3.h>
#include <Vector4.h>

namespace OriGine {
namespace ColliderMesh {

//============================================================
// 定数
//============================================================

/// AABB
static constexpr uint32_t kAabbVertexSize = 8;
static constexpr uint32_t kAabbIndexSize  = 24;

/// OBB
static constexpr uint32_t kObbVertexSize = 8;
static constexpr uint32_t kObbIndexSize  = 24;

/// Sphere
static constexpr uint32_t kSphereDivision   = 8;
static constexpr float kSphereDivisionReal  = static_cast<float>(kSphereDivision);
static constexpr uint32_t kSphereVertexSize = 4 * kSphereDivision * kSphereDivision;
static constexpr uint32_t kSphereIndexSize  = 4 * kSphereDivision * kSphereDivision;

/// Ray
static constexpr float kRayLength        = 100.0f;
static constexpr uint32_t kRayVertexSize = 2;
static constexpr uint32_t kRayIndexSize  = 2;

/// Segment
static constexpr uint32_t kSegmentVertexSize = 2;
static constexpr uint32_t kSegmentIndexSize  = 2;

/// Capsule
static constexpr uint32_t kCapsuleDivision   = 8;
static constexpr float kCapsuleDivisionReal  = static_cast<float>(kCapsuleDivision);
static constexpr uint32_t kCapsuleVertexSize = 2 + 4 * kCapsuleDivision * 2;
static constexpr uint32_t kCapsuleIndexSize  = 2 + 4 * kCapsuleDivision * 2;

//============================================================
// メッシュ生成関数テンプレート
//============================================================

/// <summary>
/// Bounds形状からラインメッシュを作成（プライマリテンプレート）
/// </summary>
template <Bounds::IsBounds ShapeType>
void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const ShapeType& _shape,
    const Vec4f& _color = kWhite) {
    // デフォルト実装：何もしない
    (void)_mesh;
    (void)_shape;
    (void)_color;
}

//============================================================
// AABB 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::AABB& _shape,
    const Vec4f& _color) {
    Vec3f shapeMin = _shape.Min();
    Vec3f shapeMax = _shape.Max();

    // AABB Vertex
    Vector3f vertexes[kAabbVertexSize]{
        {shapeMin},
        {shapeMin[X], shapeMin[Y], shapeMax[Z]},
        {shapeMax[X], shapeMin[Y], shapeMax[Z]},
        {shapeMax[X], shapeMin[Y], shapeMin[Z]},
        {shapeMin[X], shapeMax[Y], shapeMin[Z]},
        {shapeMin[X], shapeMax[Y], shapeMax[Z]},
        {shapeMax},
        {shapeMax[X], shapeMax[Y], shapeMin[Z]}};

    // AABB Index
    uint32_t indices[kAabbIndexSize]{
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7};

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    for (uint32_t vi = 0; vi < kAabbVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(vertexes[vi], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < kAabbIndexSize; ++ii) {
        _mesh->indexes_.emplace_back(startIndexesIndex + indices[ii]);
    }
}

//============================================================
// OBB 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::OBB& _shape,
    const Vec4f& _color) {
    Vector3f halfSizes  = _shape.halfSize_;
    Vector3f corners[8] = {
        {-halfSizes[X], -halfSizes[Y], -halfSizes[Z]},
        {halfSizes[X], -halfSizes[Y], -halfSizes[Z]},
        {halfSizes[X], halfSizes[Y], -halfSizes[Z]},
        {-halfSizes[X], halfSizes[Y], -halfSizes[Z]},
        {-halfSizes[X], -halfSizes[Y], halfSizes[Z]},
        {halfSizes[X], -halfSizes[Y], halfSizes[Z]},
        {halfSizes[X], halfSizes[Y], halfSizes[Z]},
        {-halfSizes[X], halfSizes[Y], halfSizes[Z]}};

    Matrix4x4 rotationMatrix = MakeMatrix4x4::RotateQuaternion(_shape.orientations_.rot);
    for (auto& corner : corners) {
        corner = corner * rotationMatrix + _shape.center_;
    }

    uint32_t indices[kObbIndexSize]{
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7};

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    for (uint32_t vi = 0; vi < kObbVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(corners[vi], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < kObbIndexSize; ++ii) {
        _mesh->indexes_.emplace_back(startIndexesIndex + indices[ii]);
    }
}

//============================================================
// Sphere 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Sphere& _shape,
    const Vec4f& _color) {
    const float kLatEvery = std::numbers::pi_v<float> / kSphereDivisionReal;
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / kSphereDivisionReal;

    auto calculatePoint = [&](float lat, float lon) -> Vector3f {
        return {
            _shape.center_[X] + _shape.radius_ * std::cos(lat) * std::cos(lon),
            _shape.center_[Y] + _shape.radius_ * std::sin(lat),
            _shape.center_[Z] + _shape.radius_ * std::cos(lat) * std::sin(lon)};
    };

    // 緯線
    for (uint32_t latIndex = 1; latIndex < kSphereDivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < kSphereDivision; ++lonIndex) {
            float lonA = lonIndex * kLonEvery;
            float lonB = (lonIndex + 1) % kSphereDivision * kLonEvery;

            Vector3f pointA = calculatePoint(lat, lonA);
            Vector3f pointB = calculatePoint(lat, lonB);

            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointA, 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointB, 1.f), _color});
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        }
    }

    // 経線
    for (uint32_t lonIndex = 0; lonIndex < kSphereDivision; ++lonIndex) {
        float lon = lonIndex * kLonEvery;
        for (uint32_t latIndex = 0; latIndex < kSphereDivision; ++latIndex) {
            float latA = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
            float latB = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

            Vector3f pointA = calculatePoint(latA, lon);
            Vector3f pointB = calculatePoint(latB, lon);

            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointA, 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(pointB, 1.f), _color});
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
            _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        }
    }
}

//============================================================
// Ray 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Ray& _shape,
    const Vec4f& _color) {
    Vector3f origin   = _shape.origin;
    Vector3f endPoint = origin + _shape.direction * kRayLength;

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(origin, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(endPoint, 1.f), _color});

    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);
}

//============================================================
// Segment 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Segment& _shape,
    const Vec4f& _color) {
    Vector3f start = _shape.start;
    Vector3f end   = _shape.end;

    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());

    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(start, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(end, 1.f), _color});

    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);
}

//============================================================
// Capsule 特殊化
//============================================================
template <>
inline void CreateLineMesh(
    Mesh<ColorVertexData>* _mesh,
    const Bounds::Capsule& _shape,
    const Vec4f& _color) {
    Vector3f start = _shape.segment.start;
    Vector3f end   = _shape.segment.end;
    float radius   = _shape.radius;

    Vector3f axis    = Vec3f(end - start);
    float axisLength = axis.length();

    // 軸が0の場合は球として描画
    if (axisLength < 0.0001f) {
        Bounds::Sphere sphere;
        sphere.center_ = start;
        sphere.radius_ = radius;
        CreateLineMesh(_mesh, sphere, _color);
        return;
    }

    Vector3f axisNorm = axis.normalize();

    Vector3f perpendicular;
    if (std::abs(axisNorm[Y]) < 0.99f) {
        perpendicular = Vec3f::Cross({0.f, 1.f, 0.f}, axisNorm).normalize();
    } else {
        perpendicular = Vec3f::Cross({1.f, 0.f, 0.f}, axisNorm).normalize();
    }
    Vector3f perpendicular2 = Vec3f::Cross(axisNorm, perpendicular).normalize();

    const float kAngleStep = 2.0f * std::numbers::pi_v<float> / kCapsuleDivisionReal;

    // 中心軸
    uint32_t startIndexesIndex = uint32_t(_mesh->vertexes_.size());
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(start, 1.f), _color});
    _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(end, 1.f), _color});
    _mesh->indexes_.emplace_back(startIndexesIndex + 0);
    _mesh->indexes_.emplace_back(startIndexesIndex + 1);

    // 始点側の円
    for (uint32_t i = 0; i < kCapsuleDivision; ++i) {
        float angle1 = i * kAngleStep;
        float angle2 = (i + 1) % kCapsuleDivision * kAngleStep;

        Vector3f p1 = start + perpendicular * (radius * std::cos(angle1)) + perpendicular2 * (radius * std::sin(angle1));
        Vector3f p2 = start + perpendicular * (radius * std::cos(angle2)) + perpendicular2 * (radius * std::sin(angle2));

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }

    // 終点側の円
    for (uint32_t i = 0; i < kCapsuleDivision; ++i) {
        float angle1 = i * kAngleStep;
        float angle2 = (i + 1) % kCapsuleDivision * kAngleStep;

        Vector3f p1 = end + perpendicular * (radius * std::cos(angle1)) + perpendicular2 * (radius * std::sin(angle1));
        Vector3f p2 = end + perpendicular * (radius * std::cos(angle2)) + perpendicular2 * (radius * std::sin(angle2));

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }

    // 両端を繋ぐ縦のライン（4本）
    for (uint32_t i = 0; i < 4; ++i) {
        float angle     = i * (std::numbers::pi_v<float> / 2.0f);
        Vector3f offset = perpendicular * (radius * std::cos(angle)) + perpendicular2 * (radius * std::sin(angle));
        Vector3f p1     = start + offset;
        Vector3f p2     = end + offset;

        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1, 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2, 1.f), _color});
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
        _mesh->indexes_.emplace_back((uint32_t)_mesh->indexes_.size());
    }
}

} // namespace ColliderMesh
} // namespace OriGine
