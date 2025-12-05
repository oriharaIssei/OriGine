#pragma once

#include "system/ISystem.h"

/// <summary>
/// エミッターの動作を管理するシステム
/// </summary>
class EmitterWorkSystem
    : public OriGine::ISystem {
public:
    EmitterWorkSystem() : ISystem(SystemCategory::Effect) {}
    ~EmitterWorkSystem() {}

    void Initialize() override;
    /*virtual void Update();*/
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;
};
