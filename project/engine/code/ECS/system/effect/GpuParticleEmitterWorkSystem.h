#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>
#include <vector>

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"

/// ECS
#include "ECS/component/effect/particle/gpuParticle/GpuParticle.h"

namespace OriGine {
/// 前方宣言
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

/// <summary>
/// GpuParticleEmitterを処理するシステム
/// GPU上でパーティクルの更新と発生を行う
/// </summary>
class GpuParticleEmitterWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    GpuParticleEmitterWorkSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~GpuParticleEmitterWorkSystem() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 全体の更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// エンティティの更新 (GpuParticleでは特別に使用しない)
    /// </summary>
    /// <param name="entity">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle /*entity*/) override {}

    /// <summary>
    /// パーティクルの更新を行う (GPUコンピュート実行)
    /// </summary>
    /// <param name="_emitter">対象のエミッター</param>
    void UpdateParticle(GpuParticleEmitter* _emitter);

    /// <summary>
    /// パーティクルの発生を行う (GPUコンピュート実行)
    /// </summary>
    /// <param name="entity">対象のエミッター</param>
    void EmitParticle(GpuParticleEmitter* entity);

protected:
    /// <summary>
    /// PSOを生成する
    /// </summary>
    void CreatePso();

    /// <summary>
    /// パーティクル発生用のPSOを生成する
    /// </summary>
    void CreateEmitGpuParticlePso();

    /// <summary>
    /// パーティクル更新用のPSOを生成する
    /// </summary>
    void CreateUpdateGpuParticlePso();

    /// <summary>
    /// コンピュートシェーダーの開始処理
    /// </summary>
    /// <param name="_pso">使用するPSO</param>
    void StartCS(PipelineStateObj* _pso);

    /// <summary>
    /// コンピュートシェーダーの実行と待機
    /// </summary>
    void ExecuteCS();

private:
    /// <summary>
    /// パーティクル発生用のPSO
    /// </summary>
    PipelineStateObj* emitGpuParticlePso_ = nullptr;

    /// <summary>
    /// パーティクル更新用のPSO
    /// </summary>
    PipelineStateObj* updateGpuParticlePso_ = nullptr;

    /// <summary>
    /// 処理対象のエミッターリスト
    /// </summary>
    std::vector<GpuParticleEmitter*> workEmitters_;

    /// <summary>
    /// DX12コマンド
    /// </summary>
    std::unique_ptr<DxCommand> dxCommand_;

    /// <summary>
    /// フレームごとの定数バッファ
    /// </summary>
    IConstantBuffer<PerFrame> perFrameBuffer_;

    /// <summary>
    /// パーティクルデータのSRVインデックス
    /// </summary>
    const int32_t particlesDataIndex = 0;

    /// <summary>
    /// フリーインデックスバッファのUAVインデックス
    /// </summary>
    const int32_t freeIndexBufferIndex = 1;

    /// <summary>
    /// フリーリストバッファのUAVインデックス
    /// </summary>
    const int32_t freeListBufferIndex = 2;

    /// <summary>
    /// エミッター形状データのSRVインデックス
    /// </summary>
    const int32_t kEmitterShapeIndex = 3;

    /// <summary>
    /// フレーム定数バッファのCBVインデックス
    /// </summary>
    const int32_t perFrameBufferIndex = 4;
};

} // namespace OriGine
