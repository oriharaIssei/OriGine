#pragma once

// lib
#include "globalVariables/SerializedField.h"

// transform
#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"

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

public:
    void setViewTranslate(const Vec3f& translate) { cameraTransform_.translate = translate; }
    const CameraTransform& getCameraTransform() const { return cameraTransform_; }
    void setFollowTarget(Transform* target) { followTarget_ = target; }
};
