#include "EffectEditScene.h"

//editor
#include "effect/particle/manager/ParticleManager.h"
//camera
#include "camera/CameraManager.h"

EffectEditScene::EffectEditScene()
    : IScene("EffectEditScene") {}

EffectEditScene::~EffectEditScene() {}

void EffectEditScene::Init() {
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    particleManager_ = ParticleManager::getInstance();
    particleManager_->Init();
}

void EffectEditScene::Update() {
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    particleManager_->Edit();
}

void EffectEditScene::Draw3d() {
}

void EffectEditScene::DrawLine() {
}

void EffectEditScene::DrawSprite() {
}

void EffectEditScene::DrawParticle() {
    particleManager_->DrawDebug();
}
