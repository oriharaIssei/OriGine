#pragma once
#include "system/ISystem.h"

/// math
#include <stdint.h>
#include <string>

namespace OriGine {

/// <summary>
/// WindowResize時のイベント登録を行うシステム
/// </summary>
class RegisterWindowResizeEvent
    : public ISystem,
      public std::enable_shared_from_this<RegisterWindowResizeEvent> {
public:
    RegisterWindowResizeEvent();
    ~RegisterWindowResizeEvent() override = default;

    /// <summary>
    /// 初期化（ウィンドウリサイズイベントの登録）
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理（登録したイベントの解除）
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// エンティティごとの更新処理（使用しない）
    /// </summary>
    /// <param name="_handle">エンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;

private:
    int32_t spriteResizeEventIndex_    = -1; // スプライトのリサイズイベントのインデックス
    int32_t sceneViewResizeEventIndex_ = -1; // シーンビューのリサイズイベントのインデックス
    int32_t subSceneResizeEventIndex_  = -1; // サブシーンのリサイズイベントのインデックス
};

} // namespace OriGine
