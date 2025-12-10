#include "CameraManager.h"

/// engine
#include "Engine.h"
// debugCamera
#include "debugCamera/DebugCamera.h"
// directX12
#include "directX12/DxDevice.h"

using namespace OriGine;

CameraManager::CameraManager() {}
CameraManager::~CameraManager() {}

void CameraManager::SetTransform(const CameraTransform& transform) {
    if (!transform.canUseMainCamera) {
        return;
    }
    cTransform_.openData_ = transform;
}

CameraManager* CameraManager::GetInstance() {
    static CameraManager instance{};
    return &instance;
}

void CameraManager::Initialize() {
    // bufferを生成
    cTransform_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    cTransform_->Initialize();
    cTransform_->UpdateMatrix();
    cTransform_.ConvertToBuffer();

#ifndef _RELEASE
    // デバッグカメラの初期化
    debugCamera_ = std::make_unique<OriGine::DebugCamera>();
    debugCamera_->Initialize();
#endif // _RELEASE
}

void CameraManager::DebugUpdate() {
    debugCamera_->Update();
    cTransform_.openData_ = debugCamera_->GetCameraTransform();
    // 情報を Buffuer に 渡す
    cTransform_.ConvertToBuffer();
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
