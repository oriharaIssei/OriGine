#pragma once

#include "IEnemy.h"

class WeakEnemy
    : public IEnemy {
public:
     void Init()   override;
     void Update() override;

     std::unique_ptr<IEnemy> Clone() override;

 private:
};
