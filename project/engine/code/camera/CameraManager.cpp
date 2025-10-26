#include "CameraManager.h"

#include "debugCamera/DebugCamera.h"
#include "directX12/DxDevice.h"
#include "Engine.h"

CameraManager::CameraManager() {}
CameraManager::~CameraManager() {}

CameraManager* CameraManager::getInstance() {
    static CameraManager instance{};
    return &instance;
}

void CameraManager::Initialize() {
    // bufferを生成
    cTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->device_);
    cTransform_->Initialize();
    cTransform_->UpdateMatrix();
    cTransform_.ConvertToBuffer();

#ifndef _RELEASE
    // デバッグカメラの初期化
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize();
#endif // _RELEASE
}

void CameraManager::DebugUpdate() {
    debugCamera_->Update();
    cTransform_.openData_ = debugCamera_->getCameraTransform();
    // 情報を Buffuer に 渡す
    cTransform_.ConvertToBuffer();
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
