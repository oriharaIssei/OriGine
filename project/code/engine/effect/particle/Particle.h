#pragma once

///stl
//memory
#include <functional>
#include <memory>
//container
#include <vector>
//string
#include <string>

///engine
//assets
#include "animation/Animation.h" // KeyFrame に関する情報
//transform
#include "transform/ParticleTransform.h"

//math
#include <stdint.h>

struct ParticleKeyFrames;

enum class ParticleUpdatePerLifeTime {
    Node,
    Scale       = 0x1 << 1,
    Rotate      = 0x1 << 2,
    Speed       = 0x1 << 3,
    Color       = 0x1 << 4,
    UvScale     = 0x1 << 5,
    UvRotate    = 0x1 << 6,
    UvTranslate = 0x1 << 7,
};

class Particle {
public:
    Particle();
    virtual ~Particle();

    void Init(
        const ParticleTransform& _initialTransfrom,
        float _lifeTime,
        const Vec3f& _direction,
        float _speed);
    void Update(float _deltaTime);

protected:
    ParticleTransform transform_;

    Vec3f direction_ = {0.0f, 0.0f, 0.0f};
    float speed_       = 0.0f;

    Vec3f velocity_ = {0.0f, 0.0f, 0.0f};

    float lifeTime_ = 0.0f;
    float currentTime_ = 0.0f;
    bool isAlive_   = false;

    ParticleKeyFrames* keyFrames_ = nullptr;
    std::vector<std::function<void()>> updateByCurbes_;

public:
    void setKeyFrames(
        int32_t updateSettings,
        ParticleKeyFrames* _keyFrames);

    const ParticleTransform& getTransform() const { return transform_; }
    bool getIsAlive() const { return isAlive_; }
};

struct ParticleKeyFrames {
    ParticleKeyFrames() {
        colorCurve_.push_back({});

        scaleCurve_.push_back({});
        rotateCurve_.push_back({});
        speedCurve_.push_back({});
        uvScaleCurve_.push_back({});
        uvRotateCurve_.push_back({});
        uvTranslateCurve_.push_back({});
    }
    ~ParticleKeyFrames() {}

    //.pkf ファイルから読み込み
    void SaveKeyFrames(const std::string& _filePath);
    void LoadKeyFrames(const std::string& _filePath);

    AnimationCurve<Vec4f> colorCurve_;

    AnimationCurve<Vec3f> scaleCurve_;
    AnimationCurve<Vec3f> rotateCurve_;
    AnimationCurve<float> speedCurve_;

    AnimationCurve<Vec3f> uvScaleCurve_;
    AnimationCurve<Vec3f> uvRotateCurve_;
    AnimationCurve<Vec3f> uvTranslateCurve_;

    // float currentTime_ = Particle::leftTime;
    // float duration     = Particle::lifeTime;
};
