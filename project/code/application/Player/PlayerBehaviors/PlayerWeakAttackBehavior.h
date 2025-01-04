#pragma once

#include "IPlayerBehavior.h"
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

private:
    SerializedField<int32_t> maxCombo_;
    int32_t currentCombo_ = 0;

    SerializedField<float> startUpTime_;
    SerializedField<float> actionTime_;
    SerializedField<float> endLagTime_;
    float currentTimer_ = 0.0f;

    std::unique_ptr<IPlayerBehavior> nextBehavior_ = nullptr;
};
