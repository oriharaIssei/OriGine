#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <vector>

namespace OriGine {

/// <summary>
/// 衝突判定システム
/// </summary>
class CollisionCheckSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CollisionCheckSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CollisionCheckSystem();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 全体の衝突判定更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// 各エンティティと他エンティティの衝突判定を行う
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;

protected:
    /// <summary>
    /// エンティティのペアを走査するためのイテレータ
    /// </summary>
    ::std::vector<EntityHandle>::iterator entityItr_;
};

} // namespace OriGine
