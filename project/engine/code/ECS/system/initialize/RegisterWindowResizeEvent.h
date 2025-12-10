#pragma once
#include "system/ISystem.h"

/// math
#include <stdint.h>

namespace OriGine {

/// <summary>
/// WindowResize時のイベント登録を行うシステム
/// </summary>
class RegisterWindowResizeEvent
    : public ISystem {
public:
    RegisterWindowResizeEvent();
    ~RegisterWindowResizeEvent() override = default;
    void Initialize() override;
    void Finalize() override;

private:
    /// <summary>
    /// 実行しない(Initializeですべてを行う)
    /// </summary>
    /// <param name="_entity"></param>
    void UpdateEntity(Entity* _entity) override;

private:
    int32_t spriteResizeEventIndex_    = -1;
    int32_t sceneViewResizeEventIndex_ = -1;
    int32_t subSceneResizeEventIndex_  = -1;
};

} // namespace OriGine
