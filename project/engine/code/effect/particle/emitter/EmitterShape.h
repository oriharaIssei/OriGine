#pragma once

/// stl
// container
#include <Array>
// string
#include <string>

// math
#include "Vector3.h"

///< summary>
/// エミッターの形状の種類
///</summary>
enum class EmitterShapeType : int32_t {
    SPHERE,
    OBB,
    Capsule,
    Cone,

    Count // 種類の数
};
static const int32_t shapeTypeCount = static_cast<int32_t>(EmitterShapeType::Count);

static std::array<std::string, shapeTypeCount> emitterShapeTypeWord_ = {
    "Sphere",
    "OBB",
    "Capsule",
    "Cone"};

///< summary>
/// エミッターの形状のどこからパーティクルを生成するか
///</summary>
enum class ParticleSpawnLocationType : int32_t {
    InBody,
    Edge,
    Count // 種類の数
};
static const int32_t particleSpawnLocationTypeCount = static_cast<int32_t>(ParticleSpawnLocationType::Count);

static std::array<std::string, shapeTypeCount> particleSpawnLocationTypeWord_ = {
    "InBody",
    "Edge"};

///< summary>
/// エミッターの形状 の Interface クラス
///</summary>
struct EmitterShape {
    EmitterShape(EmitterShapeType type)
        : type_(type) {}

public: // メンバ関数
#ifdef _DEBUG
    virtual void Debug();
#endif // _DEBUG

    virtual Vec3f getSpawnPos() = 0;

public:
    const EmitterShapeType type_;
};

///< summary>
/// Sphere 形状
///</summary>
struct EmitterSphere
    : EmitterShape {
    EmitterSphere()
        : EmitterShape(EmitterShapeType::SPHERE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG
    Vec3f getSpawnPos() override;

    float radius_;
};

///< summary>
/// Obb 形状
///</summary>
struct EmitterOBB
    : EmitterShape {
    EmitterOBB()
        : EmitterShape(EmitterShapeType::OBB) {}
// メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG

    Vec3f getSpawnPos() override;

public: // メンバ変数
    Vec3f min_;
    Vec3f max_;
    Vec3f rotate_;
};

///< summary>
/// Capsule 形状
///</summary>
struct EmitterCapsule
    : EmitterShape {
    EmitterCapsule()
        : EmitterShape(EmitterShapeType::Capsule) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG
    Vec3f getSpawnPos() override;

public: // メンバ変数
    float radius_;
    float length_;
    Vec3f direction_;
};

///< summary>
/// Cone 形状
///</summary>
struct EmitterCone
    : EmitterShape {
    EmitterCone()
        : EmitterShape(EmitterShapeType::Cone) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG

    Vec3f getSpawnPos() override;

public: // メンバ変数
    float angle_;
    float length_;
    Vec3f direction_;
};
