#include "HitEffectManager.h"

#include "Engine.h"
#include "animation/Animation.h"
#include "animation/AnimationManager.h"
#include "object3d/AnimationObject3d.h"

void HitEffectManager::Init() {
    hitEffects_.clear();
    hitEffects_.reserve(30);

    // load Check
    auto loadCheck = AnimationManager::getInstance()->Load("resource/Animations", "HitEffect.anm");
    while (true) {
        if (loadCheck->getData()) {
            break;
        }
    }
}

void HitEffectManager::Update() {
    float deltatime = Engine::getInstance()->getDeltaTime();
    hitEffects_.erase(
        std::remove_if(hitEffects_.begin(), hitEffects_.end(), [deltatime](const std::unique_ptr<AnimationObject3d>& hitEffect) {
            hitEffect->Update(deltatime);
            return hitEffect->getAnimation()->isEnd();
        }),
        hitEffects_.end());
}

void HitEffectManager::Draw() {
    for (auto& hitEffect : hitEffects_) {
        hitEffect->Draw();
    }
}

void HitEffectManager::Finalize() {
    hitEffects_.clear();
}

HitEffectManager::HitEffectManager() {
}

HitEffectManager::~HitEffectManager() {
}

void HitEffectManager::addHitEffect(const Quaternion& rotate, const Vector3& spawnTranslate) {
    hitEffects_.push_back(std::make_unique<AnimationObject3d>());
    auto& spawnedEffect = hitEffects_.back();
    spawnedEffect->Init(AnimationSetting("HitEffect"));
    spawnedEffect->transform_.rotate    = rotate;
    spawnedEffect->transform_.translate = spawnTranslate;
}
