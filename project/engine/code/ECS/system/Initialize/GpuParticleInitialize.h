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

/// <summary>
/// GpuParticle の初期化を行うシステム
/// </summary>
class GpuParticleInitialize
    : public OriGine::ISystem {
public:
    GpuParticleInitialize();
    ~GpuParticleInitialize();

    void Initialize();
    void Update();
    void Finalize();

protected:
    void UpdateEntity(Entity* _entity) override;

    void CreatePSO();
    /// <summary>
    /// コンピュートシェーダーによる初期化
    /// </summary>
    void StartCS();
    /// <summary>
    /// コンピュートシェーダーの実行
    /// </summary>
    void ExecuteCS();

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    PipelineStateObj* pso_                = nullptr;

    bool usingCS_ = false;

    const int32_t particleBufferIndex_     = 0;
    const int32_t freeIndexBufferIndex_    = 1;
    const int32_t freeListBufferIndex_     = 2;
    const int32_t emitterShapeIndex        = 3;
};
