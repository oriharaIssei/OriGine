#pragma once

//lib
#include "globalVariables/SerializedField.h"
#include "myRandom/MyRandom.h"

//transform
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

class GameCamera {
public:
    GameCamera();
    ~GameCamera();

    void Init();
    void Update();

private:
    Vec3f OffstVector();

private:
    CameraTransform cameraTransform_;

    Transform* followTarget_ = nullptr;
    SerializedField<Vec3f> followOffset_;
    Vec3f interTarget_;
    Vec2f destinationAngleXY_ = {0.0f, 0.0f};
    SerializedField<float> rotateSpeed_;
    SerializedField<float> rotateSensitivity_;

    SerializedField<float> maxRotateX_;
    SerializedField<float> minRotateX_;

    const float shakeDecay_ = 0.6f;
    float shakePower_       = 0.0f;
    Vec2f shakeDirection_   = {0.0f, 0.0f};

public:
    void Shake(float power) {
        shakePower_ = (std::max)(power, shakePower_);
        MyRandom::Float randDir = MyRandom::Float(-1.0f, 1.0f);
        shakeDirection_         = {randDir.get(), randDir.get()}; 
    }

    void setViewTranslate(const Vec3f& translate) { cameraTransform_.translate = translate; }
    const CameraTransform& getCameraTransform() const { return cameraTransform_; }
    void setFollowTarget(Transform* target) { followTarget_ = target; }
};
