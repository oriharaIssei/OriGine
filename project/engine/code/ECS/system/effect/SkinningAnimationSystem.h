#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
struct PipelineStateObj;
class DxCommand;

/// <summary>
/// SkinningAnimation を再生するシステム
/// </summary>
class SkinningAnimationSystem
    : public OriGine::ISystem {
public:
    SkinningAnimationSystem();
    ~SkinningAnimationSystem() override;

    void Initialize();
    void Update();
    void Finalize();

protected:
    void UpdateEntity(Entity* _entity) override;

    void CreatePSO();
    /// <summary>
    /// コンピュートシェーダーを開始する
    /// </summary>
    void StartCS();
    /// <summary>
    /// コンピュートシェーダーを実行する
    /// </summary>
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
