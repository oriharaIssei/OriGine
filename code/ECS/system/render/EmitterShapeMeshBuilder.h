#pragma once

/// engine
#include "directX12/mesh/DefaultVertexData.h"
#include "directX12/mesh/Mesh.h"

/// emitter shapes
#include "component/effect/particle/emitter/EmitterShape.h"

/// math
#include "math/Matrix4x4.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

/// math env
#include "math/MathEnv.h"

namespace OriGine {
namespace EmitterShapeMesh {

//============================================================
// 定数
//============================================================

static constexpr uint32_t kDivision  = 8;
static constexpr float kDivisionReal = static_cast<float>(kDivision);

/// Sphere (緯線 + 経線)
static constexpr uint32_t kSphereVertexSize = 4 * kDivision * kDivision;
static constexpr uint32_t kSphereIndexSize  = 4 * kDivision * kDivision;

/// Box (8頂点, 12辺)
static constexpr uint32_t kBoxVertexSize = 8;
static constexpr uint32_t kBoxIndexSize  = 24;

/// カプセル・コーンの側面ライン本数（0°, 90°, 180°, 270°の4方向）
static constexpr uint32_t kSideLineCount = 4;

/// Capsule (中心軸 + 両端円 + 縦ライン)
static constexpr uint32_t kCapsuleVertexSize = 2 + kSideLineCount * kDivision * 2;
static constexpr uint32_t kCapsuleIndexSize  = 2 + kSideLineCount * kDivision * 2;

/// Cone (底面円 + 頂点から底面円への側面ライン)
static constexpr uint32_t kConeDivision   = kDivision;
static constexpr uint32_t kConeVertexSize = kConeDivision * 2 + kSideLineCount * 2; // 底面円 + 側面ライン
static constexpr uint32_t kConeIndexSize  = kConeVertexSize;

/// 方向ベクトルがY軸にほぼ平行と判定する閾値
static constexpr float kNearParallelThreshold = 0.99f;

/// 方向ベクトルがほぼゼロと判定するε
static constexpr float kDirEps = 0.0001f;

//============================================================
// メッシュ生成関数
//============================================================

/// <summary>
/// EmitterSphere のラインメッシュを生成する
/// </summary>
inline void BuildSphereMesh(
    Mesh<ColorVertexData>* _mesh,
    const Vec3f& _center,
    float _radius,
    const Vec4f& _color) {

    const float kLatEvery = kPi / kDivisionReal;
    const float kLonEvery = kTau / kDivisionReal;

    auto calc = [&](float lat, float lon) -> Vec3f {
        return {
            _center[X] + _radius * std::cos(lat) * std::cos(lon),
            _center[Y] + _radius * std::sin(lat),
            _center[Z] + _radius * std::cos(lat) * std::sin(lon)};
    };

    // 緯線
    for (uint32_t latIdx = 1; latIdx < kDivision; ++latIdx) {
        float lat = -kHalfPi + kLatEvery * latIdx;
        for (uint32_t lonIdx = 0; lonIdx < kDivision; ++lonIdx) {
            Vec3f a       = calc(lat, lonIdx * kLonEvery);
            Vec3f b       = calc(lat, (lonIdx + 1) % kDivision * kLonEvery);
            uint32_t base = uint32_t(_mesh->vertexes_.size());
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(a[X], a[Y], a[Z], 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(b[X], b[Y], b[Z], 1.f), _color});
            _mesh->indexes_.push_back(base);
            _mesh->indexes_.push_back(base + 1);
        }
    }

    // 経線
    for (uint32_t lonIdx = 0; lonIdx < kDivision; ++lonIdx) {
        float lon = lonIdx * kLonEvery;
        for (uint32_t latIdx = 0; latIdx < kDivision; ++latIdx) {
            float latA    = -kHalfPi + kLatEvery * latIdx;
            float latB    = -kHalfPi + kLatEvery * (latIdx + 1);
            Vec3f a       = calc(latA, lon);
            Vec3f b       = calc(latB, lon);
            uint32_t base = uint32_t(_mesh->vertexes_.size());
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(a[X], a[Y], a[Z], 1.f), _color});
            _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(b[X], b[Y], b[Z], 1.f), _color});
            _mesh->indexes_.push_back(base);
            _mesh->indexes_.push_back(base + 1);
        }
    }
}

/// <summary>
/// EmitterBox のラインメッシュを生成する（OBB相当）
/// </summary>
inline void BuildBoxMesh(
    Mesh<ColorVertexData>* _mesh,
    const Vec3f& _origin,
    const Vec3f& _min,
    const Vec3f& _max,
    const Vec3f& _rotateEuler,
    const Vec4f& _color) {

    Vec3f halfSize    = (_max - _min) * 0.5f;
    Vec3f localCenter = (_min + _max) * 0.5f;

    Vec3f corners[8] = {
        {-halfSize[X], -halfSize[Y], -halfSize[Z]},
        {halfSize[X], -halfSize[Y], -halfSize[Z]},
        {halfSize[X], halfSize[Y], -halfSize[Z]},
        {-halfSize[X], halfSize[Y], -halfSize[Z]},
        {-halfSize[X], -halfSize[Y], halfSize[Z]},
        {halfSize[X], -halfSize[Y], halfSize[Z]},
        {halfSize[X], halfSize[Y], halfSize[Z]},
        {-halfSize[X], halfSize[Y], halfSize[Z]}};

    Matrix4x4 rot     = MakeMatrix4x4::RotateXYZ(_rotateEuler);
    Vec3f worldCenter = _origin + localCenter;
    for (auto& c : corners) {
        c = c * rot + worldCenter;
    }

    uint32_t indices[kBoxIndexSize] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7};

    uint32_t base = uint32_t(_mesh->vertexes_.size());
    for (uint32_t vi = 0; vi < kBoxVertexSize; ++vi) {
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(corners[vi][X], corners[vi][Y], corners[vi][Z], 1.f), _color});
    }
    for (uint32_t ii = 0; ii < kBoxIndexSize; ++ii) {
        _mesh->indexes_.push_back(base + indices[ii]);
    }
}

/// <summary>
/// EmitterCapsule のラインメッシュを生成する
/// </summary>
inline void BuildCapsuleMesh(
    Mesh<ColorVertexData>* _mesh,
    const Vec3f& _origin,
    const Vec3f& _direction,
    float _radius,
    float _length,
    const Vec4f& _color) {

    // 方向ベクトルがゼロに近い場合はY軸を中心軸とする
    Vec3f axisNorm = axisY;
    if (_direction.length() > kDirEps) {
        axisNorm = _direction.normalize();
    }

    Vec3f start = _origin;
    Vec3f end   = _origin + axisNorm * _length;

    Vec3f perp;
    if (std::abs(axisNorm[Y]) < kNearParallelThreshold) {
        perp = Vec3f::Cross({0.f, 1.f, 0.f}, axisNorm).normalize();
    } else {
        perp = Vec3f::Cross({1.f, 0.f, 0.f}, axisNorm).normalize();
    }
    Vec3f perp2 = Vec3f::Cross(axisNorm, perp).normalize();

    const float kStep = kTau / kDivisionReal;

    // 中心軸
    {
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(start[X], start[Y], start[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(end[X], end[Y], end[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }

    // 始点側の円
    for (uint32_t i = 0; i < kDivision; ++i) {
        float a1      = i * kStep;
        float a2      = (i + 1) % kDivision * kStep;
        Vec3f p1      = start + perp * (_radius * std::cos(a1)) + perp2 * (_radius * std::sin(a1));
        Vec3f p2      = start + perp * (_radius * std::cos(a2)) + perp2 * (_radius * std::sin(a2));
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1[X], p1[Y], p1[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2[X], p2[Y], p2[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }

    // 終点側の円
    for (uint32_t i = 0; i < kDivision; ++i) {
        float a1      = i * kStep;
        float a2      = (i + 1) % kDivision * kStep;
        Vec3f p1      = end + perp * (_radius * std::cos(a1)) + perp2 * (_radius * std::sin(a1));
        Vec3f p2      = end + perp * (_radius * std::cos(a2)) + perp2 * (_radius * std::sin(a2));
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1[X], p1[Y], p1[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2[X], p2[Y], p2[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }

    // 縦ライン
    for (uint32_t i = 0; i < kSideLineCount; ++i) {
        float angle   = i * kHalfPi;
        Vec3f offset  = perp * (_radius * std::cos(angle)) + perp2 * (_radius * std::sin(angle));
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        Vec3f so = start + offset;
        Vec3f eo = end + offset;
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(so[X], so[Y], so[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(eo[X], eo[Y], eo[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }
}

/// <summary>
/// EmitterCone のラインメッシュを生成する
/// </summary>
inline void BuildConeMesh(
    Mesh<ColorVertexData>* _mesh,
    const Vec3f& _origin,
    const Vec3f& _direction,
    float _angle,
    float _length,
    const Vec4f& _color) {

    Vec3f axisNorm = _direction.length() > kDirEps ? _direction.normalize() : Vec3f{0.f, 0.f, 1.f};
    float baseRadius = _length * std::tan(_angle);
    Vec3f baseCenter = _origin + axisNorm * _length;

    Vec3f perp;
    if (std::abs(axisNorm[Y]) < kNearParallelThreshold) {
        perp = Vec3f::Cross({0.f, 1.f, 0.f}, axisNorm).normalize();
    } else {
        perp = Vec3f::Cross({1.f, 0.f, 0.f}, axisNorm).normalize();
    }
    Vec3f perp2 = Vec3f::Cross(axisNorm, perp).normalize();

    const float kStep = kTau / static_cast<float>(kConeDivision);

    // 底面円
    for (uint32_t i = 0; i < kConeDivision; ++i) {
        float a1      = i * kStep;
        float a2      = (i + 1) % kConeDivision * kStep;
        Vec3f p1      = baseCenter + perp * (baseRadius * std::cos(a1)) + perp2 * (baseRadius * std::sin(a1));
        Vec3f p2      = baseCenter + perp * (baseRadius * std::cos(a2)) + perp2 * (baseRadius * std::sin(a2));
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p1[X], p1[Y], p1[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(p2[X], p2[Y], p2[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }

    // 側面ライン（頂点 → 底面円）
    for (uint32_t i = 0; i < kSideLineCount; ++i) {
        float angle   = i * kHalfPi;
        Vec3f tip     = _origin;
        Vec3f rim     = baseCenter + perp * (baseRadius * std::cos(angle)) + perp2 * (baseRadius * std::sin(angle));
        uint32_t base = uint32_t(_mesh->vertexes_.size());
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(tip[X], tip[Y], tip[Z], 1.f), _color});
        _mesh->vertexes_.emplace_back(ColorVertexData{Vec4f(rim[X], rim[Y], rim[Z], 1.f), _color});
        _mesh->indexes_.push_back(base);
        _mesh->indexes_.push_back(base + 1);
    }
}

} // namespace EmitterShapeMesh
} // namespace OriGine
