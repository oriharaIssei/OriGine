#include "ISystem.h"

void ISystem::Update() {
#ifdef _DEBUG
    // 計測開始
    deltaTimer_.Initialize();
#endif

    if (entities_.empty()) {
        return;
    }
    eraseDeadEntity();

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }

#ifdef _DEBUG
    // 計測終了
    deltaTimer_.Update();
    runningTime_ = deltaTimer_.getDeltaTime();
#endif
}

void ISystem::Edit() {

}
