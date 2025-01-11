#include "EffectEditScene.h"

#include "particle/manager/ParticleManager.h"

EffectEditScene::EffectEditScene()
    : IScene("EffectEditScene") {}

EffectEditScene::~EffectEditScene() {}

void EffectEditScene::Init() {
    particleManager_ = std::make_unique<ParticleManager>();
    particleManager_->Init();
}

void EffectEditScene::Update() {
    particleManager_->Edit();
}

void EffectEditScene::Draw3d() {
}

void EffectEditScene::DrawLine() {
}

void EffectEditScene::DrawSprite() {
}

void EffectEditScene::DrawParticle() {
}
