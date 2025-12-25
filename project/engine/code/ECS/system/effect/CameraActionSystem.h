#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// CameraAction を再生するシステム
/// </summary>
class CameraActionSystem
    : public ISystem {
public:
    CameraActionSystem();
    ~CameraActionSystem();

    void Initialize();
    void Finalize();

protected:
    void UpdateEntity(EntityHandle _handle);
};

} // namespace OriGine
