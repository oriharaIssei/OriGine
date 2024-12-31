#include "CameraManager.h"

#include "Engine.h"

CameraManager::CameraManager()
    : IModule() {}
CameraManager::~CameraManager() {}

CameraManager* CameraManager::getInstance() {
    static CameraManager instance{};
    return &instance;
}

void CameraManager::Init() {
    cTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
