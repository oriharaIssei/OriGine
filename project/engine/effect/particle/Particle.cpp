#include "Particle.h"

///stl
//io
#include <fstream>
#include <iostream>

///engine
//transform
#include "component/transform/ParticleTransform.h"

// lib
#include "myRandom/MyRandom.h"

Particle::Particle() {}

Particle::~Particle() {}

void Particle::Init(
    const ParticleTransform& _initialTransfrom,
    const Vec3f& _minVelocity,
    const Vec3f& _maxVelocity,
    const Vec3f& _minScale,
    const Vec3f& _maxScale,
    const Vec3f& _minRotate,
    const Vec3f& _maxRotate,
    float _lifeTime,
    const Vec3f& _direction,
    const Vec3f& _velocity) {
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
    Vec3f rotatedVelocity = RotateVector(velocity_, rotation);

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
            float angle       = std::acos(dot);
            Quaternion q      = Quaternion::RotateAxisAngle(axis, angle).normalize();
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
        updateByCurves_.push_back([this]() {
            transform_.color = CalculateValue::LINEAR(keyFrames_->colorCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime)) {
        updateByCurves_.push_back([this]() {
            transform_.scale = CalculateValue::LINEAR(keyFrames_->scaleCurve_, currentTime_) * scaleRatio_;
        });
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::ScaleRandom)) {
        MyRandom::Float randomX(minUpdateScale_->v[X], maxUpdateScale_->v[X]);
        MyRandom::Float randomY(minUpdateScale_->v[Y], maxUpdateScale_->v[Y]);
        MyRandom::Float randomZ(minUpdateScale_->v[Z], maxUpdateScale_->v[Z]);
        transform_.scale += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime)) {
        updateByCurves_.push_back([this]() {
            transform_.rotate = CalculateValue::LINEAR(keyFrames_->rotateCurve_, currentTime_);
        });
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotateRandom)) {
        MyRandom::Float randomX(minUpdateRotate_->v[X], maxUpdateRotate_->v[X]);
        MyRandom::Float randomY(minUpdateRotate_->v[Y], maxUpdateRotate_->v[Y]);
        MyRandom::Float randomZ(minUpdateRotate_->v[Z], maxUpdateRotate_->v[Z]);
        transform_.rotate += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::RotateForward)) {
        rotateForward_ = true;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) {
        updateByCurves_.push_back([this]() {
            velocity_ = CalculateValue::LINEAR(keyFrames_->velocityCurve_, currentTime_);
        });
    } else if (updateSettings & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) {
        MyRandom::Float randomX(minUpdateVelocity_->v[X], maxUpdateVelocity_->v[X]);
        MyRandom::Float randomY(minUpdateVelocity_->v[Y], maxUpdateVelocity_->v[Y]);
        MyRandom::Float randomZ(minUpdateVelocity_->v[Z], maxUpdateVelocity_->v[Z]);
        velocity_ += Vec3f(randomX.get(), randomY.get(), randomZ.get()) * deltaTime_;
    }

    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) {
        updateByCurves_.push_back([this]() {
            transform_.uvScale = CalculateValue::LINEAR(keyFrames_->uvScaleCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) {
        updateByCurves_.push_back([this]() {
            transform_.uvRotate = CalculateValue::LINEAR(keyFrames_->uvRotateCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) {
        updateByCurves_.push_back([this]() {
            transform_.uvTranslate = CalculateValue::LINEAR(keyFrames_->uvTranslateCurve_, currentTime_);
        });
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
void ParticleKeyFrames::SaveKeyFrames(const std::string& _filePath) {
    /*
    1. fileを開く
    2. colorCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    3. scaleCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    4. rotateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    5. speedCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    6. uvScaleCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    7. uvRotateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    8. uvTranslateCurve_のkeyframesを書き込む(サイズ, 各キーフレーム)
    9. fileを閉じる
    */

    //1. fileを開く
    std::ofstream ofs(_filePath, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing");
    }

    auto writeCurve = [&ofs](const auto& curve) {
        size_t size = curve.size();
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const auto& keyframe : curve) {
            ofs.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));
            ofs.write(reinterpret_cast<const char*>(&keyframe.value), sizeof(keyframe.value));
        }
    };
    // 2 ~ 8 書き込み
    writeCurve(colorCurve_);
    writeCurve(scaleCurve_);
    writeCurve(rotateCurve_);
    writeCurve(velocityCurve_);
    writeCurve(uvScaleCurve_);
    writeCurve(uvRotateCurve_);
    writeCurve(uvTranslateCurve_);

    //9. fileを閉じる
    ofs.close();
}

void ParticleKeyFrames::LoadKeyFrames(const std::string& _filePath) {
    /*
    1. fileを開く
    2. colorCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    3. scaleCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    4. rotateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    5. speedCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    6. uvScaleCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    7. uvRotateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    8. uvTranslateCurve_のkeyframesを読み込む(サイズ, 各キーフレーム)
    9. fileを閉じる
    */
    //1. fileを開く
    std::ifstream ifs(_filePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for reading");
    }
    auto readCurve = [&ifs](auto& curve) {
        size_t size;
        ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
        curve.resize(size);
        for (auto& keyframe : curve) {
            ifs.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time));
            ifs.read(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value));
        }
    };
    // 2 ~ 8 読み込み
    readCurve(colorCurve_);
    readCurve(scaleCurve_);
    readCurve(rotateCurve_);
    readCurve(velocityCurve_);
    readCurve(uvScaleCurve_);
    readCurve(uvRotateCurve_);
    readCurve(uvTranslateCurve_);
    //9. fileを閉じる
    ifs.close();
}
#pragma endregion
