#include "ISystem.h"

void ISystem::Update() {
    if (entities_.empty()) {
        return;
    }
    eraseDeadEntity();

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }
}
