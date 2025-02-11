#include "RenderManager.h"

RenderManager::RenderManager() {
}

RenderManager::~RenderManager() {
}

void RenderManager::Init() {
    if (renderControllers_.empty()) {
        renderControllers_.clear();
    }
}

void RenderManager::RenderFrame() {
    for (auto& [name, controller] : renderControllers_) {
        controller->RenderFrame(); // 更新と描画をひとまとめにした関数
    }
}

void RenderManager::Finalize() {
    for (auto& [name, controller] : renderControllers_) {
        controller->Finalize();
    }
    renderControllers_.clear();
}
