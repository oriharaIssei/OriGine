#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
struct PipelineStateObj;
class DxCommand;

class SkinningAnimationSystem
    : public ISystem {
public:
    SkinningAnimationSystem();
    ~SkinningAnimationSystem() override;

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

    // rootParameter indices
    const int32_t outputVertexBufferIndex_         = 0;
    const int32_t inputVertexBufferIndex_          = 1;
    const int32_t matrixPaletteBufferIndex_        = 2;
    const int32_t vertexInfluenceBufferIndex_      = 3;
    const int32_t gSkinningInformationBufferIndex_ = 4;
};
