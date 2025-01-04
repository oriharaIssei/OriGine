#include "GameCamera.h"

#include "Vector3.h"
#include "input/Input.h"

GameCamera::GameCamera()
    : followOffset_("Game", "GameCamera", "followOffset"),
      rotateSpeed_("Game", "GameCamera", "rotateSpeed"),
      rotateSensitivity_("Game", "GameCamera", "rotateSensitivity_") {}

GameCamera::~GameCamera() {}

void GameCamera::Init() {
    cameraTransform_.translate = {0.0f, 0.0f, -10.0f};
    cameraTransform_.rotate    = {0.0f, 0.0f, 0.0f};
}

void GameCamera::Update() {
    if (followTarget_) {
        cameraTransform_.rotate.y = std::lerp(cameraTransform_.rotate.y, destinationAngleY_, rotateSensitivity_);

        Vector3 offset = OffstVector();
        interTarget_   = Lerp(followTarget_->translate, interTarget_, rotateSensitivity_);

        cameraTransform_.translate = offset + followTarget_->translate;
    }
}

Vector3 GameCamera::OffstVector() {
    return TransformVector(followOffset_, MakeMatrix::RotateXYZ(cameraTransform_.rotate));
}
