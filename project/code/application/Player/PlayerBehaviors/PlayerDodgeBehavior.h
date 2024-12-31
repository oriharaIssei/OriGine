#pragma once

#include "IPlayerBehavior.h"

#include "Vector3.h"
#include "globalVariables/SerializedField.h"

class PlayerDodgeBehavior
    : public IPlayerBehavior {
public:
    PlayerDodgeBehavior(Player* _player, const Vector3& direction);
    ~PlayerDodgeBehavior();
    void Init() override;
    void Update() override;

protected:
    void StartUp() override; // 使わない
    void Action() override;
    void EndLag() override;

private:
    SerializedField<float> actionTime_;
    SerializedField<float> endLagTime_;
    SerializedField<float> distance_;
    float currentTimer_ = 0.0f;
    // 行動前と その後の 座標
    Vector3 beforePos_;
    Vector3 afterPos_;
};
