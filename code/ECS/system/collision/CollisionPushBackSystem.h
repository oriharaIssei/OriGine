#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// 衝突判定後に押し戻し処理を行うシステム
/// </summary>
class CollisionPushBackSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CollisionPushBackSystem()
        : ISystem(SystemCategory::Collision) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CollisionPushBackSystem() {}

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
    /// エンティティの押し戻し処理を行う
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
