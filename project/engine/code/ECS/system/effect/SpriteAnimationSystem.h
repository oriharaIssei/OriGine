#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// スプライトアニメーションを再生・制御するシステム
/// </summary>
class SpriteAnimationSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    SpriteAnimationSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~SpriteAnimationSystem() override;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// 各エンティティのスプライトアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
