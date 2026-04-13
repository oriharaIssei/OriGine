#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// CameraAction を再生するシステム
/// </summary>
class CameraActionSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CameraActionSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CameraActionSystem();

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
    /// 各エンティティのカメラアクションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
