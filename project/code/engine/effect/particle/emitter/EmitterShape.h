#pragma once

///stl
//container
#include <Array>
//string
#include <string>

///engine
//lib
#include "globalVariables/SerializedField.h"

//math
#include "Vector3.h"

///<summary>
///エミッターの形状の種類
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

///<summary>
///エミッターの形状のどこからパーティクルを生成するか
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

///<summary>
///エミッターの形状 の Interface クラス
///</summary>
struct EmitterShape {
    EmitterShape(EmitterShapeType type, const std::string& scene, const std::string& emitterName)
        : type_(type),
          spawnType_{scene, emitterName, emitterShapeTypeWord_[int(type)] + "spawnLocationType"} {}

public: // メンバ関数
    virtual void Debug();

    virtual Vec3f getSpawnPos() = 0;

protected:
    SerializedField<int32_t> spawnType_;
    const EmitterShapeType type_;
};

///<summary>
/// Sphere 形状
///</summary>
struct EmitterSphere
    : EmitterShape {
    EmitterSphere(const std::string& scene, const std::string& emitterName)
        : EmitterShape(EmitterShapeType::SPHERE, scene, emitterName),
          radius_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_radius"} {}

public: // メンバ関数
    void Debug() override;
    Vec3f getSpawnPos() override;

public: // メンバ変数
    SerializedField<float> radius_;
};

///<summary>
/// Obb 形状
///</summary>
struct EmitterOBB
    : EmitterShape {
    EmitterOBB(const std::string& scene, const std::string& emitterName)
        : EmitterShape(EmitterShapeType::OBB, scene, emitterName),
          min_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_min"},
          max_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_max"},
          rotate_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_rotate"} {}
// メンバ関数
    void Debug() override;

    Vec3f getSpawnPos() override;

private: // メンバ変数
    SerializedField<Vec3f> min_;
    SerializedField<Vec3f> max_;
    SerializedField<Vec3f> rotate_;
};

///<summary>
/// Capsule 形状
///</summary>
struct EmitterCapsule
    : EmitterShape {
    EmitterCapsule(const std::string& scene, const std::string& emitterName)
        : EmitterShape(EmitterShapeType::Capsule, scene, emitterName),
          radius_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_radius"},
          length_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_length"},
          direction_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_direction"} {}

public: // メンバ関数
    void Debug() override;
    Vec3f getSpawnPos() override;

private: // メンバ変数
    SerializedField<float> radius_;
    SerializedField<float> length_;
    SerializedField<Vec3f> direction_;
};

///<summary>
/// Cone 形状
///</summary>
struct EmitterCone
    : EmitterShape {
    EmitterCone(const std::string& scene, const std::string& emitterName)
        : EmitterShape(EmitterShapeType::Cone, scene, emitterName),
          angle_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_angle"},
          length_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_length"},
          direction_{scene, emitterName, emitterShapeTypeWord_[int(type_)] + "_direction"} {}

public: // メンバ関数
    void Debug() override;

    Vec3f getSpawnPos() override;

private: // メンバ変数
    SerializedField<float> angle_;
    SerializedField<float> length_;
    SerializedField<Vec3f> direction_;
};
