#pragma once
#include "system/ISystem.h"

#include <array>
#include <memory>

/// engine
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/effect/MaterialEffectPipeLine.h"
/// system
#include "system/postRender/DissolveEffect.h"
#include "system/postRender/DistortionEffect.h"
#include "system/postRender/GradationEffect.h"

namespace OriGine {

/// <summary>
/// Material にPostEffectをかけるためのSystem
/// </summary>
class MaterialEffect
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    MaterialEffect();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~MaterialEffect() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// マテリアルエフェクトの更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// 使用していない (個別にUpdateで行う)
    /// </summary>
    /// <param name="_owner">エンティティハンドル</param>
    void UpdateEntity(EntityHandle /*_owner*/) override {}

    /// <summary>
    /// コンポーネントの振り分け
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void DispatchComponents(EntityHandle _handle);

    /// <summary>
    /// エフェクトパイプラインの更新
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    /// <param name="_pipeline">対象のパイプラインコンポーネント</param>
    void UpdateEffectPipeline(EntityHandle _handle, MaterialEffectPipeLine* _pipeline);

    /// <summary>
    /// TextureにEffectをかける
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    /// <param name="_type">エフェクトの種類</param>
    /// <param name="_output">出力先テクスチャ</param>
    void TextureEffect(EntityHandle _handle, MaterialEffectType _type, RenderTexture* _output);

    /// <summary>
    /// コマンドの実行
    /// </summary>
    void ExecuteCommand();

private:
    int32_t currentTempRTIndex_ = 0;

    std::array<std::unique_ptr<RenderTexture>, 2> tempRenderTextures_ = {nullptr, nullptr};

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    DxDsvDescriptor dxDsv_{};
    std::unique_ptr<DxResource> dsvResource_ = nullptr;

    std::vector<std::pair<EntityHandle, MaterialEffectPipeLine*>> effectPipelines_;

    std::unique_ptr<DissolveEffect> dissolveEffect_     = nullptr;
    std::unique_ptr<DistortionEffect> distortionEffect_ = nullptr;
    std::unique_ptr<GradationEffect> gradationEffect_   = nullptr;
};

} // namespace OriGine
