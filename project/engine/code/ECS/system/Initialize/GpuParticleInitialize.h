#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <cstdint>
#include <memory>

namespace OriGine {
//// 前方宣言
/// engine
// directX12 Object
class DxCommand;
struct PipelineStateObj;

/// <summary>
/// GpuParticle の初期化を行うシステム
/// </summary>
class GpuParticleInitialize
    : public ISystem {
public:
    GpuParticleInitialize();
    ~GpuParticleInitialize();

    void Initialize();
    void Update();
    void Finalize();

protected:
    void UpdateEntity(EntityHandle _handle) override;

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

    const int32_t kParticleBufferIndex_  = 0;
    const int32_t kFreeIndexBufferIndex_ = 1;
    const int32_t kFreeListBufferIndex_  = 2;
    const int32_t kEmitterShapeIndex     = 3;
};

} // namespace OriGine
