#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// シーン初期化時にカメラを指定したCameraTransformで初期化するシステム
/// </summary>
class CameraInitialize
    : public ISystem {
public:
    CameraInitialize();
    ~CameraInitialize() override;

    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(Entity* _entity) override;
};

} // namespace OriGine
