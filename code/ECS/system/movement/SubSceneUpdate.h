#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// SubSceneコンポーネントを保持するエンティティを通じて、サブシーンのライフサイクルと更新を管理するシステム
/// </summary>
class SubSceneUpdate
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    SubSceneUpdate() : ISystem(SystemCategory::Movement, 0) {}

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override {}

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override {}

private:
    /// <summary>
    /// 各エンティティが持つサブシーンを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
