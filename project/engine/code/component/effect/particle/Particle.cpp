#include "Particle.h"

/// stl
// io
#include <fstream>
#include <iostream>

/// engine
// transform
#include "component/transform/ParticleTransform.h"

// lib
#include "binaryIO/BinaryIO.h"
#include "myRandom/MyRandom.h"

Particle::Particle() {}

Particle::~Particle() {}

void Particle::Initialize(
    const ParticleTransform& _initialTransfrom,
    const Vec3f& _minVelocity,
    const Vec3f& _maxVelocity,
    const Vec3f& _minScale,
    const Vec3f& _maxScale,
    const Vec3f& _minRotate,
    const Vec3f& _maxRotate,
    float _lifeTime,
    const Vec3f& _direction,
    const Vec3f& _velocity,
    InterpolationType _transform,
    InterpolationType _color,
    InterpolationType _uv) {
    transform_ = _initialTransfrom;
    transform_.UpdateMatrix();

    direction_ = _direction;
    velocity_  = _velocity;

    isAlive_ = true;

    scaleRatio_    = transform_.scale / (_maxScale - _minScale);
    rotateRatio_   = transform_.rotate / (_maxRotate - _minRotate);
    velocityRatio_ = velocity_ / (_maxVelocity - _minVelocity);

    lifeTime_    = _lifeTime;
    currentTime_ = 0.0f;

    transformInterpolationType_ = _transform;
    colorInterpolationType_     = _color;
    uvInterpolationType_        = _uv;
}

void Particle::Update(float _deltaTime) {
    deltaTime_ = _deltaTime;
    if (!isAlive_) {
        return;
    }
    currentTime_ += deltaTime_;
    if (currentTime_ >= lifeTime_) {
        isAlive_ = false;
        return;
    }

    for (auto& update : updateByCurves_) {
        update();
    }

    // direction_ を法線ベクトルとして velocity_ を回転させる
    Vec3f rotationAxis    = axisZ.cross(direction_).normalize();
    float angle           = std::acos(Vec3f(axisZ * direction_).dot() / (axisZ.length() * direction_.length()));
    Quaternion rotation   = Quaternion::RotateAxisAngle(rotationAxis, angle);
    Vec3f rotatedVelocity = Quaternion::RotateVector(velocity_, rotation);

    // 回転させた velocity_ で移動
    Vec3f movement = rotatedVelocity * _deltaTime;
    transform_.translate += movement;

    if (rotateForward_) {
        // 進行方向を向くように回転させる
        if (rotatedVelocity.length() < 0.0f) {
            rotatedVelocity = direction_;
        }
        Vec3f forward = rotatedVelocity.normalize();
        float dot     = Vec3f(axisZ * forward).dot();
        if (dot < 1.0f - std::numeric_limits<float>::epsilon()) {
            Vec3f axis        = axisZ.cross(forward).normalize();
            float rotateAngle = std::acos(dot);
            Quaternion q      = Quaternion::RotateAxisAngle(axis, rotateAngle).normalize();
            transform_.rotate = q.ToEulerAngles();
        }
    }
    transform_.UpdateMatrix();
}

void Particle::setKeyFrames(int32_t updateSettings, ParticleKeyFrames* _keyFrames) {
    if (updateSettings == 0 || !_keyFrames) {
        return;
    }

    keyFrames_ = _keyFrames;

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) {
        if (colorInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.color = CalculateValue::Linear(keyFrames_->colorCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.color = CalculateValue::Step(keyFrames_->colorCurve_, currentTime_);
            });
        }
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime)) {
        if (transformInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.scale = CalculateValue::Linear(keyFrames_->scaleCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.scale = CalculateValue::Step(keyFrames_->scaleCurve_, currentTime_);
            });
        }
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::ScaleRandom)) {
        MyRandom::Float randomX(minUpdateScale_->v[X], maxUpdateScale_->v[X]);
        MyRandom::Float randomY(minUpdateScale_->v[Y], maxUpdateScale_->v[Y]);
        MyRandom::Float randomZ(minUpdateScale_->v[Z], maxUpdateScale_->v[Z]);
        transform_.scale += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime)) {
        if (transformInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.rotate = CalculateValue::Linear(keyFrames_->rotateCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.rotate = CalculateValue::Step(keyFrames_->rotateCurve_, currentTime_);
            });
        }
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotateRandom)) {
        MyRandom::Float randomX(minUpdateRotate_->v[X], maxUpdateRotate_->v[X]);
        MyRandom::Float randomY(minUpdateRotate_->v[Y], maxUpdateRotate_->v[Y]);
        MyRandom::Float randomZ(minUpdateRotate_->v[Z], maxUpdateRotate_->v[Z]);
        transform_.rotate += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotateForward)) {
        rotateForward_ = true;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) {
        if (transformInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                velocity_ = CalculateValue::Linear(keyFrames_->velocityCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                velocity_ = CalculateValue::Step(keyFrames_->velocityCurve_, currentTime_);
            });
        }
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) {
        MyRandom::Float randomX(minUpdateVelocity_->v[X], maxUpdateVelocity_->v[X]);
        MyRandom::Float randomY(minUpdateVelocity_->v[Y], maxUpdateVelocity_->v[Y]);
        MyRandom::Float randomZ(minUpdateVelocity_->v[Z], maxUpdateVelocity_->v[Z]);
        velocity_ += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) {
        if (uvInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.uvScale = CalculateValue::Linear(keyFrames_->uvScaleCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.uvScale = CalculateValue::Step(keyFrames_->uvScaleCurve_, currentTime_);
            });
        }
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) {
        if (uvInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.uvRotate = CalculateValue::Linear(keyFrames_->uvRotateCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.uvRotate = CalculateValue::Step(keyFrames_->uvRotateCurve_, currentTime_);
            });
        }
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) {
        if (uvInterpolationType_ == InterpolationType::LINEAR) {
            updateByCurves_.push_back([this]() {
                transform_.uvTranslate = CalculateValue::Linear(keyFrames_->uvTranslateCurve_, currentTime_);
            });
        } else {
            updateByCurves_.push_back([this]() {
                transform_.uvTranslate = CalculateValue::Step(keyFrames_->uvTranslateCurve_, currentTime_);
            });
        }
    }
}

void Particle::UpdateKeyFrameValues() {
    auto updateCurve = [](auto& curve, const auto& min, const auto& max) {
        for (auto& keyframe : curve) {
            for (int i = 0; i < 3; ++i) {
                float range = max.v[i] - min.v[i];
                if (range != 0) {
                    float ratio         = (keyframe.value.v[i] - min.v[i]) / range;
                    keyframe.value.v[i] = min.v[i] + ratio * range;
                }
            }
        }
    };

    if (minUpdateScale_ && maxUpdateScale_) {
        updateCurve(keyFrames_->scaleCurve_, *minUpdateScale_, *maxUpdateScale_);
    }
    if (minUpdateRotate_ && maxUpdateRotate_) {
        updateCurve(keyFrames_->rotateCurve_, *minUpdateRotate_, *maxUpdateRotate_);
    }
    if (minUpdateVelocity_ && maxUpdateVelocity_) {
        updateCurve(keyFrames_->velocityCurve_, *minUpdateVelocity_, *maxUpdateVelocity_);
    }
}

#pragma region "ParticleKeyFrames"
void ParticleKeyFrames::SaveKeyFrames(BinaryWriter& _writer) const {
    /*
        2. colorCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        3. scaleCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        4. rotateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        5. speedCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        6. uvScaleCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        7. uvRotateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
        8. uvTranslateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
     */

    // 2 ~ 8 書き込み
    WriteCurve("uvTranslateCurve", uvTranslateCurve_, _writer);
    WriteCurve("colorCurve", colorCurve_, _writer);
    WriteCurve("scaleCurve", scaleCurve_, _writer);
    WriteCurve("rotateCurve", rotateCurve_, _writer);
    WriteCurve("velocityCurve", velocityCurve_, _writer);
    WriteCurve("uvScaleCurve", uvScaleCurve_, _writer);
    WriteCurve("uvRotateCurve", uvRotateCurve_, _writer);
}

void ParticleKeyFrames::LoadKeyFrames(BinaryReader& _reader) {
    /*
        2. colorCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        3. scaleCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        4. rotateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        5. speedCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        6. uvScaleCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        7. uvRotateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
        8. uvTranslateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    */

    // 2 ~ 8 読み込み
    ReadCurve("colorCurve", colorCurve_, _reader);
    ReadCurve("scaleCurve", scaleCurve_, _reader);
    ReadCurve("rotateCurve", rotateCurve_, _reader);
    ReadCurve("velocityCurve", velocityCurve_, _reader);
    ReadCurve("uvScaleCurve", uvScaleCurve_, _reader);
    ReadCurve("uvRotateCurve", uvRotateCurve_, _reader);
    ReadCurve("uvTranslateCurve", uvTranslateCurve_, _reader);
}
#pragma endregion
