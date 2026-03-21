#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// ParticleSystem の動作を管理するシステム
/// </summary>
class ParticleSystemWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    ParticleSystemWorkSystem() : ISystem(SystemCategory::Effect) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~ParticleSystemWorkSystem() {}

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
    /// 各エンティティのエミッターを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
