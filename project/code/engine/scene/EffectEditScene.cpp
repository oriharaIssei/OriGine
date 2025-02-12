#include "EffectEditScene.h"

//editor
#include "effect/editor/EffectEditor.h"
#include "effect/particle/editor/ParticleEditor.h"
//camera
#include "camera/CameraManager.h"

EffectEditScene::EffectEditScene()
    : IScene("EffectEditScene") {}

EffectEditScene::~EffectEditScene() {}

void EffectEditScene::Init() {
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    effectEditor_ = std::make_unique<EffectEditor>();
    effectEditor_->Init();

    particleEditor_ = std::make_unique<ParticleEditor>();
    particleEditor_->Init();
}

void EffectEditScene::Update() {
    // ================== camera ==================//
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    // ================== Editor ==================//
    effectEditor_->Update();
    particleEditor_->Update();
}

void EffectEditScene::Draw3d() {
}

void EffectEditScene::DrawLine() {
}

void EffectEditScene::DrawSprite() {
}

void EffectEditScene::DrawParticle() {
    effectEditor_->Draw();
    particleEditor_->Draw();
}
