#pragma once

#include "system/ISystem.h"

class CameraInitialize
    : public ISystem {
public:
    CameraInitialize();
    ~CameraInitialize() override;

    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(GameEntity* _entity) override;
};
