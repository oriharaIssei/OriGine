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

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

protected:
    /// <summary>
    /// エンティティの更新（パーティクルエミッターの初期化）
    /// </summary>
    /// <param name="_handle">エンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO();

    /// <summary>
    /// コンピュートシェーダー(CS)による初期化開始
    /// </summary>
    void StartCS();

    /// <summary>
    /// コンピュートシェーダー(CS)の実行
    /// </summary>
    void ExecuteCS();

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr; // DirectXコマンド管理
    PipelineStateObj* pso_                = nullptr; // パイプラインステートオブジェクト

    bool usingCS_ = false; // CSを使用中かどうか

    // ルートパラメーターのインデックス
    const int32_t kParticleBufferIndex_  = 0;
    const int32_t kFreeIndexBufferIndex_ = 1;
    const int32_t kFreeListBufferIndex_  = 2;
    const int32_t kEmitterShapeIndex     = 3;
};

} // namespace OriGine
