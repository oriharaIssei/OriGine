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
    /// <summary>
    /// コンストラクタ
    /// </summary>
    SkinningAnimationSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~SkinningAnimationSystem() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// 各エンティティのスキニングアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;

    /// <summary>
    /// PSOを生成する
    /// </summary>
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

    // rootParameter indices
    const int32_t kOutputVertexBufferIndex_        = 0;
    const int32_t kInputVertexBufferIndex_         = 1;
    const int32_t kMatrixPaletteBufferIndex_       = 2;
    const int32_t kVertexInfluenceBufferIndex_     = 3;
    const int32_t kSkinningInformationBufferIndex_ = 4;
};

} // namespace OriGine
