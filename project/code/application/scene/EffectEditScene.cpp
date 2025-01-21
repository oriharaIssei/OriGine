#include "EffectEditScene.h"

//editor
#include "effect/manager/EffectManager.h"
//camera
#include "camera/CameraManager.h"

EffectEditScene::EffectEditScene()
    : IScene("EffectEditScene") {}

EffectEditScene::~EffectEditScene() {}

void EffectEditScene::Init() {
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    EffectManager_ = EffectManager::getInstance();
    EffectManager_->Init();
}

void EffectEditScene::Update() {
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    EffectManager_->Edit();
}

void EffectEditScene::Draw3d() {
}

void EffectEditScene::DrawLine() {
}

void EffectEditScene::DrawSprite() {
}

void EffectEditScene::DrawParticle() {
    EffectManager_->DrawDebug();
}
