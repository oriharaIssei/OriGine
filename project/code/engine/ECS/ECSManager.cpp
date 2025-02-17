#include "ECSManager.h"

void EntityComponentSystemManager::Init() {
    // エンティティの初期化
    entities_.resize(entityCapacity_);
    for (uint32_t i = 0; i < entityCapacity_; ++i) {
        freeEntityIndex_.push_back(i);
    }
}

void EntityComponentSystemManager::Run() {
    // システムの更新
    for (auto& system : systems_) {
        system->Update();
    }
}
