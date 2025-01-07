#pragma once

#include "Quaternion.h"
#include "Vector3.h"

#include <memory>
#include <vector>

class AnimationObject3d;

class HitEffectManager {
public:
    static HitEffectManager* getInstance() {
        static HitEffectManager instance;
        return &instance;
    }

    void Init();
    void Update();
    void Draw();
    void Finalize();

private:
    HitEffectManager();
    ~HitEffectManager();
    HitEffectManager(const HitEffectManager&)            = delete;
    HitEffectManager& operator=(const HitEffectManager&) = delete;

    std::vector<std::unique_ptr<AnimationObject3d>> hitEffects_;

public:
    void addHitEffect(const Quaternion& rotate, const Vector3& spawnTranslate);
};
