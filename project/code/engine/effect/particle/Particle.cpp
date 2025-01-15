#include "Particle.h"

///stl
//io
#include <fstream>
#include <iostream>

///engine
//transform
#include "transform/ParticleTransform.h"

Particle::Particle() {}

Particle::~Particle() {}

void Particle::Init(const ParticleTransform& _initialTransfrom, float _lifeTime, const Vector3& _direction, float _speed) {
    transform_ = _initialTransfrom;
    transform_.UpdateMatrix();

    direction_ = _direction;
    speed_     = _speed;
    velocity_  = direction_ * speed_;

    isAlive_ = true;

    lifeTime_    = _lifeTime;
    currentTime_ = 0.0f;
}

void Particle::Update(float _deltaTime) {
    if (!isAlive_) {
        return;
    }

    currentTime_ += _deltaTime;
    if (currentTime_ >= lifeTime_) {
        isAlive_ = false;
        return;
    }

    for (auto& update : updateByCurbes_) {
        update();
    }

    transform_.translate += velocity_ * _deltaTime;

    transform_.UpdateMatrix();
}

void Particle::setKeyFrames(int32_t updateSettings, ParticleKeyFrames* _keyFrames) {
    if (updateSettings == 0 ||! _keyFrames) {
        return;
    }

    keyFrames_ = _keyFrames;

    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::Color)) {
        updateByCurbes_.push_back([this]() {
            transform_.color = CalculateValue(keyFrames_->colorCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::Scale)) {
        updateByCurbes_.push_back([this]() {
            transform_.scale = CalculateValue(keyFrames_->scaleCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::Rotate)) {
        updateByCurbes_.push_back([this]() {
            transform_.rotate = CalculateValue(keyFrames_->rotateCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::Speed)) {
        updateByCurbes_.push_back([this]() {
            speed_    = CalculateValue(keyFrames_->speedCurve_, currentTime_);
            velocity_ = direction_ * speed_;
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::UvScale)) {
        updateByCurbes_.push_back([this]() {
            transform_.uvScale = CalculateValue(keyFrames_->uvScaleCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::UvRotate)) {
        updateByCurbes_.push_back([this]() {
            transform_.uvRotate = CalculateValue(keyFrames_->uvRotateCurve_, currentTime_);
        });
    }
    if (updateSettings & static_cast<int32_t>(ParticleUpdatePerLifeTime::UvTranslate)) {
        updateByCurbes_.push_back([this]() {
            transform_.uvTranslate = CalculateValue(keyFrames_->uvTranslateCurve_, currentTime_);
        });
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
    writeCurve(speedCurve_);
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
    readCurve(speedCurve_);
    readCurve(uvScaleCurve_);
    readCurve(uvRotateCurve_);
    readCurve(uvTranslateCurve_);
    //9. fileを閉じる
    ifs.close();
}
#pragma endregion
