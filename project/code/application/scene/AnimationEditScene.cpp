#include "AnimationEditScene.h"

#include "camera/CameraManager.h"

AnimationEditScene::AnimationEditScene()
    : IScene("AnimationEditScene") {}

AnimationEditScene::~AnimationEditScene() {}

void AnimationEditScene::Init() {
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();
    debugCamera_->setViewRotate(Vector3(0.3f, 3.14f, 0.0f));
    debugCamera_->setViewTranslate(Vector3(0.0f, 2.0f, 10.0f));

    animationEditor_ = std::make_unique<AnimationEditor>();
    animationEditor_->Init();

    //===================== Ground =====================//
    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models/CheckBoard", "CheckBoard.obj");
    ground_->transform_.translate.v[Y] = -10.0f;
    ground_->transform_.UpdateMatrix();
    ground_->UpdateTransform();
}

void AnimationEditScene::Update() {
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    animationEditor_->Update();
}

void AnimationEditScene::Draw3d() {
    ground_->Draw();

    animationEditor_->DrawEditObject();
}
void AnimationEditScene::DrawLine() {}
void AnimationEditScene::DrawSprite() {}
void AnimationEditScene::DrawParticle() {}
