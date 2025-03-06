#include "CameraManager.h"

#include "debugCamera/DebugCamera.h"
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

    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();
}

void CameraManager::DebugUpdate() {
    debugCamera_->Update();

    cTransform_->UpdateMatrix();
    // DebugCamera の 更新情報を 渡す
    cTransform_->viewMat = debugCamera_->getCameraTransform().viewMat;
    // 情報を Buffuer に 渡す
    cTransform_.ConvertToBuffer();
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
