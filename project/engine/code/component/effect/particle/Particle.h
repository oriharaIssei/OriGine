#pragma once

/// stl
// memory
#include <functional>
#include <memory>
// container
#include <vector>
// string
#include <string>

/// engine
// assets
#include "component/animation/ModelNodeAnimation.h" // KeyFrame に関する情報
// transform
#include "component/transform/ParticleTransform.h"

// math
#include <stdint.h>

class BinaryWriter;
class BinaryReader;

struct ParticleKeyFrames;

enum class ParticleUpdateType {
    None                   = 0,
    ScalePerLifeTime       = 0x1 << 1,
    ScaleRandom            = 0x1 << 2,
    RotatePerLifeTime      = 0x1 << 3,
    RotateRandom           = 0x1 << 4,
    VelocityPerLifeTime    = 0x1 << 5,
    VelocityRandom         = 0x1 << 6,
    ColorPerLifeTime       = 0x1 << 7,
    UvScalePerLifeTime     = 0x1 << 8,
    UvRotatePerLifeTime    = 0x1 << 9,
    UvTranslatePerLifeTime = 0x1 << 10,

    RotateForward = 0x1 << 11,
};

class Particle {
public:
    Particle();
    virtual ~Particle();

    void Initialize(
        const ParticleTransform& _initialTransfrom,
        const Vec3f& _minVelocity,
        const Vec3f& _maxVelocity,
        const Vec3f& _minScale,
        const Vec3f& _maxScale,
        const Vec3f& _minRotate,
        const Vec3f& _maxRotate,
        float _lifeTime,
        const Vec3f& _direction,
        const Vec3f& _velocity);
    void Update(float _deltaTime);
    void UpdateKeyFrameValues();

protected:
    ParticleTransform transform_;

    Vec3f direction_    = {0.0f, 0.0f, 1.0f};
    Vec3f velocity_     = {0.0f, 0.0f, 0.0f};
    bool rotateForward_ = false;

    float deltaTime_ = 0.0f;

    float lifeTime_    = 0.0f;
    float currentTime_ = 0.0f;
    bool isAlive_      = false;

    ParticleKeyFrames* keyFrames_ = nullptr;
    std::vector<std::function<void()>> updateByCurves_;
    Vec3f scaleRatio_;
    Vec3f rotateRatio_;
    Vec3f velocityRatio_;

    Vec3f* minUpdateScale_    = nullptr;
    Vec3f* maxUpdateScale_    = nullptr;
    Vec3f* minUpdateRotate_   = nullptr;
    Vec3f* maxUpdateRotate_   = nullptr;
    Vec3f* minUpdateVelocity_ = nullptr;
    Vec3f* maxUpdateVelocity_ = nullptr;

public:
    void setKeyFrames(
        int32_t updateSettings,
        ParticleKeyFrames* _keyFrames);

    const ParticleTransform& getTransform() const { return transform_; }
    bool getIsAlive() const { return isAlive_; }

    void setUpdateScaleMinMax(Vec3f* _min, Vec3f* _max) {
        minUpdateScale_ = _min;
        maxUpdateScale_ = _max;
    }
    void setUpdateRotateMinMax(Vec3f* _min, Vec3f* _max) {
        minUpdateRotate_ = _min;
        maxUpdateRotate_ = _max;
    }
    void setUpdateVelocityMinMax(Vec3f* _min, Vec3f* _max) {
        minUpdateVelocity_ = _min;
        maxUpdateVelocity_ = _max;
    }
};

struct ParticleKeyFrames {
    ParticleKeyFrames() {
        colorCurve_.push_back({});

        scaleCurve_.push_back({});
        rotateCurve_.push_back({});
        velocityCurve_.push_back({});
        uvScaleCurve_.push_back({});
        uvRotateCurve_.push_back({});
        uvTranslateCurve_.push_back({});
    }
    ~ParticleKeyFrames() {}

    //.pkf ファイルから読み込み
    void SaveKeyFrames(BinaryWriter& _writer) const;
    void LoadKeyFrames(BinaryReader& _reader);

    AnimationCurve<Vec4f> colorCurve_;

    AnimationCurve<Vec3f> scaleCurve_;
    AnimationCurve<Vec3f> rotateCurve_;
    AnimationCurve<Vec3f> velocityCurve_;

    AnimationCurve<Vec3f> uvScaleCurve_;
    AnimationCurve<Vec3f> uvRotateCurve_;
    AnimationCurve<Vec3f> uvTranslateCurve_;

    // float currentTime_ = Particle::leftTime;
    // float duration     = Particle::lifeTime;
};
