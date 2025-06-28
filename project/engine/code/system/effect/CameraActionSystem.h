#pragma once
#include "system/ISystem.h"

class CameraActionSystem
    : public ISystem {
public:
    CameraActionSystem();
    ~CameraActionSystem();

    void Initialize();
    void Finalize();

protected:
    void UpdateEntity(GameEntity* _entity);
};
