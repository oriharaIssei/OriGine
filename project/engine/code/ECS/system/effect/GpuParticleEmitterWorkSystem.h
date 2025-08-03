#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// directX12
#include "directX12/IConstantBuffer.h"

class DxCommand;
struct PipelineStateObj;

struct PerFrame {
    float time      = 0.0f;
    float deltaTime = 0.0f;

    struct ConstantBuffer {
        float time      = 0.0f;
        float deltaTime = 0.0f;

        ConstantBuffer& operator=(const PerFrame& other) {
            time      = other.time;
            deltaTime = other.deltaTime;
            return *this;
        }
    };
};

class GpuParticleEmitterWorkSystem
    : public ISystem {
public:
    GpuParticleEmitterWorkSystem();
    ~GpuParticleEmitterWorkSystem() override;

    void Initialize() override;
    void Update() override;
    void Finalize();

    void UpdateEntity(GameEntity* entity) override;
    void EmitParticle(GameEntity* entity);

protected:
    void CreatePso();
    void CreateEmitGpuParticlePso();
    void CreateUpdateGpuParticlePso();

    void StartCS(PipelineStateObj* _pso);

    void ExecuteCS();

private:
    PipelineStateObj* emitGpuParticlePso_   = nullptr;
    PipelineStateObj* updateGpuParticlePso_ = nullptr;

    std::unique_ptr<DxCommand> dxCommand_;

    IConstantBuffer<PerFrame> perFrameBuffer_;

    // particlesData と perFrame は Emitと Updateの両方で使用されるため 並んでいたほうが都合がいい
    // freeList , freeIndex も
    const int32_t particlesDataIndex   = 0;
    const int32_t freeIndexBufferIndex = 1;
    const int32_t freeListBufferIndex  = 2;
    const int32_t perFrameBufferIndex  = 3;
    const int32_t emitterShapeIndex    = 4;
};
