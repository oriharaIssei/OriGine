#pragma once

/// stl
// container
#include <Array>
// string
#include <string>

// math
#include "Vector3.h"

// binaryIO
class BinaryWriter;
class BinaryReader;

///< summary>
/// エミッターの形状の種類
///</summary>
enum class EmitterShapeType : int32_t {
    SPHERE,
    OBB,
    CAPSULE,
    CONE,

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
    EmitterShape(EmitterShapeType _shapeType)
        : type_(_shapeType) {}

public: // メンバ関数
#ifdef _DEBUG
    virtual void Debug();
#endif // _DEBUG

    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;

    virtual Vec3f getSpawnPos() = 0;

public:
    const EmitterShapeType type_;
    ParticleSpawnLocationType spawnType_ = ParticleSpawnLocationType::InBody;
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

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

public: // メンバ変数
    float radius_ = 0;
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

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

public: // メンバ変数
    Vec3f min_    = {0.f, 0.f, 0.f};
    Vec3f max_    = {0.f, 0.f, 0.f};
    Vec3f rotate_ = {0.f, 0.f, 0.f};
};

///< summary>
/// Capsule 形状
///</summary>
struct EmitterCapsule
    : EmitterShape {
    EmitterCapsule()
        : EmitterShape(EmitterShapeType::CAPSULE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG
    Vec3f getSpawnPos() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

public: // メンバ変数
    float radius_    = 0.f;
    float length_    = 0.f;
    Vec3f direction_ = {0.f, 0.f, 0.f};
};

///< summary>
/// Cone 形状
///</summary>
struct EmitterCone
    : EmitterShape {
    EmitterCone()
        : EmitterShape(EmitterShapeType::CONE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug() override;
#endif // _DEBUG

    Vec3f getSpawnPos() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

public: // メンバ変数
    float angle_     = 0.f;
    float length_    = 0.f;
    Vec3f direction_ = {0.f, 0.f, 0.f};
};
