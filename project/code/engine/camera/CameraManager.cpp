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
    cTransform_->Init();
    cTransform_->UpdateMatrix();
    cTransform_.ConvertToBuffer();
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
