#pragma once

#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// シーン初期化時にカメラを指定したCameraTransformで初期化するシステム
/// </summary>
class CameraInitialize
    : public ISystem {
public:
    CameraInitialize();
    ~CameraInitialize() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// エンティティの更新（カメラの初期化）
    /// </summary>
    /// <param name="_handle">エンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
