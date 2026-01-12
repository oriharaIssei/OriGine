#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// PrimitiveNodeAnimationのアニメーション更新を行うシステム
/// </summary>
class PrimitiveNodeAnimationWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    PrimitiveNodeAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~PrimitiveNodeAnimationWorkSystem() override = default;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override {}

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override {}

protected:
    /// <summary>
    /// 各エンティティのプリミティブノードアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
