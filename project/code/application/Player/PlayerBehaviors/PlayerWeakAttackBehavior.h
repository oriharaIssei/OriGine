#pragma once

//parent
#include "IPlayerBehavior.h"
//lib
#include "globalVariables/SerializedField.h"

class PlayerWeakAttackBehavior
    : public IPlayerBehavior {
public:
    PlayerWeakAttackBehavior(Player* _player, int32_t currentCombo);
    ~PlayerWeakAttackBehavior();
    void Init() override;
    void Update() override;

protected:
    void StartUp() override;
    void Action() override;
    void EndLag() override;

    void CreateAttackCollider();
    void CheckCombo();

private:
    // Combo
    SerializedField<int32_t> maxCombo_;
    int32_t currentCombo_ = 0;

    // Timer
    SerializedField<float> startUpTime_;
    SerializedField<float> actionTime_;
    SerializedField<float> endLagTime_;
    float currentTimer_ = 0.0f;

    // invisibleTime
    SerializedField<float> hitDetectionTimeForAttack_;

    // HitStop
    SerializedField<float> hitStopScale_;
    SerializedField<float> hitStopTime_;

    // Attack
    SerializedField<float> attackPower_;
    SerializedField<Vec3f> attackColliderOffset_;

    SerializedField<float> knockBackPower_;

    Vec3f lastDirection_;
    SerializedField<float> rotateInterpolateInStartup_;
    SerializedField<Vec3f> velocityInStartup_;
    float speedInStartup_     = 0.0f;
    Vec3f directionInStartup_ = Vec3f(0.0f, 0.0f, 0.0f);

    SerializedField<float> rotateInterpolateInAttack_;
    SerializedField<Vec3f> velocityInAttack_;
    float speedInAttack_     = 0.0f;
    Vec3f directionInAttack_ = Vec3f(0.0f, 0.0f, 0.0f);

    std::unique_ptr<IPlayerBehavior> nextBehavior_ = nullptr;
};
