#pragma once
#include "system/ISystem.h"

/// <summary>
/// CameraAction を再生するシステム
/// </summary>
class CameraActionSystem
    : public OriGine::ISystem {
public:
    CameraActionSystem();
    ~CameraActionSystem();

    void Initialize();
    void Finalize();

protected:
    void UpdateEntity(Entity* _entity);
};
