#pragma once

#include "IEnemy.h"

#include "globalVariables/SerializedField.h"

class WeakEnemy
    : public IEnemy {
public:
    WeakEnemy();
    ~WeakEnemy();

     void Init()   override;
     void Update() override;

     std::unique_ptr<IEnemy> Clone() override;

 private:
     SerializedField<float> minPlayer2Distance_;
     SerializedField<float> maxPlayer2Distance_;
     // playerとの distance
     float player2Distance_ = 0.0f;
};
