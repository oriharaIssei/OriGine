#include "AnimationEditScene.h"

#include "camera/CameraManager.h"

AnimationEditScene::AnimationEditScene():IScene("AnimationEditScene"){}

AnimationEditScene::~AnimationEditScene(){}

void AnimationEditScene::Init(){
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    animationEditor_ = std::make_unique<AnimationEditor>();
    animationEditor_->Init();
}

void AnimationEditScene::Update(){
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    animationEditor_->Update();
}

void AnimationEditScene::Draw3d(){
    animationEditor_->DrawEditObject();
}
void AnimationEditScene::DrawLine(){}
void AnimationEditScene::DrawSprite(){}
void AnimationEditScene::DrawParticle(){}
