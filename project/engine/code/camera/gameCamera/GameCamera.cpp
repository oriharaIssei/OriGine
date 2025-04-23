#include "GameCamera.h"

#include "input/Input.h"
#include "Vector3.h"

#include <algorithm>

GameCamera::GameCamera()
    : followOffset_("Game", "GameCamera", "followOffset"),
      rotateSpeed_("Game", "GameCamera", "rotateSpeed"),
      rotateSensitivity_("Game", "GameCamera", "rotateSensitivity_"),
      maxRotateX_("Game", "GameCamera", "maxRotateX"),
      minRotateX_("Game", "GameCamera", "minRotateX") {}

GameCamera::~GameCamera() {}

void GameCamera::Initialize() {
    cameraTransform_.translate = {0.0f, 0.0f, -10.0f};
    cameraTransform_.rotate    = Quaternion();
}

void GameCamera::Update() {
    Input* input = Input::getInstance();
    if (followTarget_) {
        Vec2f rotateVelocity;
        if (input->isPadActive()) {
            rotateVelocity = input->getRStickVelocity() * rotateSpeed_;
            // input の x,yをそれぞれの角度に変換
            destinationAngleXY_ += {-rotateVelocity[Y], rotateVelocity[X]};
        }

        destinationAngleXY_[X] = std::clamp(destinationAngleXY_[X], minRotateX_.operator float(), maxRotateX_.operator float());

        cameraTransform_.rotate[X] = std::lerp(cameraTransform_.rotate[X], destinationAngleXY_[X], rotateSensitivity_);
        cameraTransform_.rotate[Y] = std::lerp(cameraTransform_.rotate[Y], destinationAngleXY_[Y], rotateSensitivity_);

        Vec3f offset = OffstVector();
        interTarget_ = Lerp(followTarget_->translate, interTarget_, rotateSensitivity_);

        cameraTransform_.translate = offset + interTarget_;
    }
    cameraTransform_.UpdateMatrix();
}

Vec3f GameCamera::OffstVector() {
    return TransformVector(followOffset_, MakeMatrix::RotateQuaternion(cameraTransform_.rotate));
}
