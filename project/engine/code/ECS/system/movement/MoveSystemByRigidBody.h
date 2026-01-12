#pragma once
// parent
#include "system/ISystem.h"

/// util
#include "util/globalVariables/SerializedField.h"

namespace OriGine {

/// <summary>
/// Rigidbodyコンポーネントによる物理挙動（移動・加速度・速度）をTransformに反映するシステム
/// </summary>
class MoveSystemByRigidBody
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    MoveSystemByRigidBody();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~MoveSystemByRigidBody();

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
    /// 各エンティティのRigidbodyに基づいた物理移動を計算し、Transformを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;

protected:
    /// <summary>
    /// 重力加速度の設定値
    /// </summary>
    SerializedField<float> gravity_ = SerializedField<float>("Settings", "Physics", "Gravity");
};

} // namespace OriGine
