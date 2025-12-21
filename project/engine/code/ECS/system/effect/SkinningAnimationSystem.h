#pragma once
#include "system/ISystem.h"

/// stl
#include <memory>

namespace OriGine {
//// 前方宣言
/// engine
// directX12Object
struct PipelineStateObj;
class DxCommand;

/// <summary>
/// SkinningAnimation を再生するシステム
/// </summary>
class SkinningAnimationSystem
    : public ISystem {
public:
    SkinningAnimationSystem();
    ~SkinningAnimationSystem() override;

    void Initialize();
    void Update();
    void Finalize();

protected:
    void UpdateEntity(EntityHandle _handle) override;

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
    const int32_t kOutputVertexBufferIndex_         = 0;
    const int32_t kInputVertexBufferIndex_          = 1;
    const int32_t kMatrixPaletteBufferIndex_        = 2;
    const int32_t kVertexInfluenceBufferIndex_      = 3;
    const int32_t kSkinningInformationBufferIndex_ = 4;
};

} // namespace OriGine
