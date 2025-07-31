#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <cstdint>
#include <memory>

/// engine
// directX12 Object
class DxCommand;
struct PipelineStateObj;

class GpuParticleInitialize
    : public ISystem {
public:
    GpuParticleInitialize();
    ~GpuParticleInitialize();

    void Initialize();
    void Update();
    void Finalize();

protected:
    void UpdateEntity(GameEntity* _entity) override;

    void CreatePSO();
    void StartCS();
    void ExecuteCS();

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    PipelineStateObj* pso_                = nullptr;

    bool usingCS_ = false;

    int32_t particleBufferIndex_ = 0;
};
