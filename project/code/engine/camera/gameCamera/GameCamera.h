#pragma once

//lib
#include "globalVariables/SerializedField.h"

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
    Vector3 OffstVector();

private:
    CameraTransform cameraTransform_;

    Transform* followTarget_ = nullptr;
    SerializedField<Vector3> followOffset_;
    Vector3 interTarget_;
    float destinationAngleY_ = 0.0f;
    SerializedField<float> rotateSpeed_;
    SerializedField<float> rotateSensitivity_;

public:
    void setViewTranslate(const Vector3& translate) { cameraTransform_.translate = translate; }
    const CameraTransform& getCameraTransform() const { return cameraTransform_; }
    void setFollowTarget(Transform* target) { followTarget_ = target; }
};
