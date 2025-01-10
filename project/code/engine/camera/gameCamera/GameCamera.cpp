#include "GameCamera.h"

#include "Vector3.h"
#include "input/Input.h"

#include <algorithm>

GameCamera::GameCamera()
    : followOffset_("Game", "GameCamera", "followOffset"),
      rotateSpeed_("Game", "GameCamera", "rotateSpeed"),
      rotateSensitivity_("Game", "GameCamera", "rotateSensitivity_"),
      maxRotateX_("Game", "GameCamera", "maxRotateX"),
      minRotateX_("Game", "GameCamera", "minRotateX") {}

GameCamera::~GameCamera() {}

void GameCamera::Init() {
    cameraTransform_.translate = {0.0f, 0.0f, -10.0f};
    cameraTransform_.rotate    = {0.0f, 0.0f, 0.0f};
}

void GameCamera::Update() {
    Input* input = Input::getInstance();
    if (followTarget_) {
        Vec2f rotateVelocity;
        if (input->isPadActive()) {
            rotateVelocity = input->getRStickVelocity() * rotateSpeed_;
            // input の x,yをそれぞれの角度に変換
            destinationAngleXY_ += {-rotateVelocity.y(), rotateVelocity.x()};
        }

        destinationAngleXY_.x() = std::clamp(destinationAngleXY_.x(), minRotateX_.operator float(), maxRotateX_.operator float());

        cameraTransform_.rotate.x() = std::lerp(cameraTransform_.rotate.x(), destinationAngleXY_.x(), rotateSensitivity_);
        cameraTransform_.rotate.y() = std::lerp(cameraTransform_.rotate.y(), destinationAngleXY_.y(), rotateSensitivity_);

        Vec3f offset = OffstVector();
        interTarget_   = Lerp(followTarget_->translate, interTarget_, rotateSensitivity_);

        cameraTransform_.translate = offset + interTarget_;
    }
    cameraTransform_.UpdateMatrix();
}

Vec3f GameCamera::OffstVector() {
    return TransformVector(followOffset_, MakeMatrix::RotateXYZ(cameraTransform_.rotate));
}
