#pragma once

/// stl
// container
#include <Array>
// string
#include <string>
/// math
#include "Vector3.h"

namespace OriGine {

///< summary>
/// エミッターの形状の種類
///</summary>
enum class EmitterShapeType : int32_t {
    SPHERE,
    BOX,
    CAPSULE,
    CONE,

    Count // 種類の数
};
static const int32_t kShapeTypeCount = static_cast<int32_t>(EmitterShapeType::Count);

static std::array<std::string, kShapeTypeCount> kEmitterShapeTypeWord = {
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
static const int32_t kParticleSpawnLocationTypeCount = static_cast<int32_t>(ParticleSpawnLocationType::Count);

static std::array<std::string, kShapeTypeCount> kParticleSpawnLocationTypeWord = {
    "InBody",
    "Edge"};

///< summary>
/// エミッターの形状 の Interface クラス
///</summary>
struct EmitterShape {
    EmitterShape(EmitterShapeType _shapeType)
        : type(_shapeType) {}

public: // メンバ関数
#ifdef _DEBUG
    virtual void Debug([[maybe_unused]] const std::string& _parentLabel);
#endif // _DEBUG

    virtual Vec3f GetSpawnPos() = 0;

public:
    const EmitterShapeType type;
    ParticleSpawnLocationType spawnType = ParticleSpawnLocationType::InBody;
};

///< summary>
/// Sphere 形状
///</summary>
struct EmitterSphere
    : EmitterShape {
    friend void to_json(nlohmann::json& _j, const EmitterSphere& _comp) {
        _j["radius"]    = _comp.radius_;
        _j["spawnType"] = (int32_t)_comp.spawnType;
    }
    friend void from_json(const nlohmann::json& _j, EmitterSphere& _comp) {
        _j.at("radius").get_to(_comp.radius_);
        if (_j.find("spawnType") != _j.end()) {
            int32_t spawnType = 0;
            _j.at("spawnType").get_to(spawnType);
            _comp.spawnType = static_cast<ParticleSpawnLocationType>(spawnType);
        }
    }

    EmitterSphere()
        : EmitterShape(EmitterShapeType::SPHERE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug([[maybe_unused]] const std::string& _parentLabel) override;
#endif // _DEBUG
    Vec3f GetSpawnPos() override;

public: // メンバ変数
    float radius_ = 0;
};

///< summary>
/// Obb 形状
///</summary>
struct EmitterBox
    : EmitterShape {
    friend void to_json(nlohmann::json& _j, const EmitterBox& _comp) {
        _j["min"]       = _comp.min_;
        _j["max"]       = _comp.max_;
        _j["rotate"]    = _comp.rotate_;
        _j["spawnType"] = (int32_t)_comp.spawnType;
    }
    friend void from_json(const nlohmann::json& _j, EmitterBox& _comp) {
        _j.at("min").get_to(_comp.min_);
        _j.at("max").get_to(_comp.max_);
        _j.at("rotate").get_to(_comp.rotate_);

        if (_j.find("spawnType") != _j.end()) {
            int32_t spawnType = 0;
            _j.at("spawnType").get_to(spawnType);
            _comp.spawnType = static_cast<ParticleSpawnLocationType>(spawnType);
        }
    }

    EmitterBox()
        : EmitterShape(EmitterShapeType::BOX) {}

// メンバ関数
#ifdef _DEBUG
    void Debug([[maybe_unused]] const std::string& _parentLabel) override;
#endif // _DEBUG

    Vec3f GetSpawnPos() override;

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
    friend void to_json(nlohmann::json& _j, const EmitterCapsule& _comp) {
        _j["direction"] = _comp.direction_;
        _j["radius"]    = _comp.radius_;
        _j["length"]    = _comp.length_;

        _j["spawnType"] = (int32_t)_comp.spawnType;
    }
    friend void from_json(const nlohmann::json& _j, EmitterCapsule& _comp) {
        _j.at("direction").get_to(_comp.direction_);
        _j.at("radius").get_to(_comp.radius_);
        _j.at("length").get_to(_comp.length_);

        if (_j.find("spawnType") != _j.end()) {
            int32_t spawnType = 0;
            _j.at("spawnType").get_to(spawnType);
            _comp.spawnType = static_cast<ParticleSpawnLocationType>(spawnType);
        }
    }

    EmitterCapsule()
        : EmitterShape(EmitterShapeType::CAPSULE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug([[maybe_unused]] const std::string& _parentLabel) override;
#endif // _DEBUG
    Vec3f GetSpawnPos() override;

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
    friend void to_json(nlohmann::json& _j, const EmitterCone& _comp) {
        _j["angle"]     = _comp.angle_;
        _j["length"]    = _comp.length_;
        _j["direction"] = _comp.direction_;

        _j["spawnType"] = (int32_t)_comp.spawnType;
    }
    friend void from_json(const nlohmann::json& _j, EmitterCone& _comp) {
        _j.at("angle").get_to(_comp.angle_);
        _j.at("length").get_to(_comp.length_);
        _j.at("direction").get_to(_comp.direction_);

        if (_j.find("spawnType") != _j.end()) {
            int32_t spawnType = 0;
            _j.at("spawnType").get_to(spawnType);
            _comp.spawnType = static_cast<ParticleSpawnLocationType>(spawnType);
        }
    }

    EmitterCone()
        : EmitterShape(EmitterShapeType::CONE) {}

public: // メンバ関数
#ifdef _DEBUG
    void Debug([[maybe_unused]] const std::string& _parentLabel) override;
#endif // _DEBUG

    Vec3f GetSpawnPos() override;

public: // メンバ変数
    float angle_     = 0.f;
    float length_    = 0.f;
    Vec3f direction_ = {0.f, 0.f, 0.f};
};

} // namespace OriGine
