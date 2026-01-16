#include "CameraManager.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"
// debugCamera
#include "debugCamera/DebugCamera.h"
// directX12
#include "directX12/DxDevice.h"

/// external
#include "logger/Logger.h"

using namespace OriGine;

CameraManager::CameraManager() {}
CameraManager::~CameraManager() {}

CameraManager* CameraManager::GetInstance() {
    static CameraManager instance;
    return &instance;
}

void CameraManager::Initialize() {
    cameraBuffers_.clear();
}

void CameraManager::Finalize() {
    for (auto& [key, buffer] : cameraBuffers_) {
        buffer.Finalize();
    }
    cameraBuffers_.clear();
}

bool CameraManager::RegisterSceneCamera(const std::string& _sceneName) {
    if (cameraBuffers_.contains(_sceneName)) {
        LOG_WARN("Scene camera already registered: {}", _sceneName);
        return false;
    }

    auto& buffer = cameraBuffers_[_sceneName];
    buffer.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    buffer->Initialize(nullptr, EntityHandle());
    buffer->UpdateMatrix();
    buffer.ConvertToBuffer();

    return true;
}

bool CameraManager::UnregisterSceneCamera(const std::string& _sceneName) {
    auto itr = cameraBuffers_.find(_sceneName);
    if (itr == cameraBuffers_.end()) {
        LOG_WARN("Cannot unregister scene camera: Scene is not registered. \n SceneName : {}", _sceneName);
        return false;
    }

    itr->second.Finalize();
    cameraBuffers_.erase(itr);
    return true;
}

bool CameraManager::RegisterSceneCamera(Scene* _scene) {
    if (_scene == nullptr) {
        LOG_WARN("Cannot register scene camera: Scene is nullptr.");
        return false;
    }
    return RegisterSceneCamera(_scene->GetName());
}

bool CameraManager::UnregisterSceneCamera(Scene* _scene) {
    if (_scene == nullptr) {
        LOG_WARN("Cannot unregister scene camera: Scene is nullptr.");
        return false;
    }
    return UnregisterSceneCamera(_scene->GetName());
}

bool CameraManager::SetTransform(
    const std::string& _sceneName,
    const CameraTransform& _transform) {

    if (!_transform.canUseMainCamera) {
        LOG_WARN("Cannot set transform: CameraTransform cannot use main camera.");
        return false;
    }

    auto itr = cameraBuffers_.find(_sceneName);
    if (itr == cameraBuffers_.end()) {
        LOG_WARN("Cannot set transform: Scene is not registered. \n ScenenName : {}", _sceneName);
        return false;
    }

    itr->second.openData_ = _transform;
    return true;
}

bool CameraManager::SetTransform(Scene* _scene, const CameraTransform& _transform) {
    if (_scene == nullptr) {
        LOG_WARN("Cannot set transform: Scene is nullptr.");
        return false;
    }
    return SetTransform(_scene->GetName(), _transform);
}

bool CameraManager::DataConvertToBuffer(const std::string& _sceneName) {
    if (!cameraBuffers_.contains(_sceneName)) {
        LOG_WARN("Scene is not registered. \n SceneName : {}", _sceneName);
        return false;
    }

    cameraBuffers_[_sceneName].ConvertToBuffer();
    return true;
}

bool CameraManager::DataConvertToBuffer(Scene* _scene) {
    if (_scene == nullptr) {
        LOG_WARN("Cannot convert data to buffer: Scene is nullptr.");
        return false;
    }
    return DataConvertToBuffer(_scene->GetName());
}

bool CameraManager::SetBufferForRootParameter(
    const std::string& _sceneName,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList,
    uint32_t _rootParameterNum) {
    if (!cameraBuffers_.contains(_sceneName)) {
        LOG_WARN("Scene is not registered. \n SceneName : {}", _sceneName);
        return false;
    }

    cameraBuffers_[_sceneName].SetForRootParameter(
        _cmdList, _rootParameterNum);
    return true;
}

bool CameraManager::SetBufferForRootParameter(Scene* _scene, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, uint32_t _rootParameterNum) {
    if (_scene == nullptr) {
        LOG_WARN("Cannot set buffer for root parameter: Scene is nullptr.");
        return false;
    }
    return SetBufferForRootParameter(_scene->GetName(), _cmdList, _rootParameterNum);
}

CameraTransform CameraManager::GetTransform(const std::string& _sceneName) const {
    auto itr = cameraBuffers_.find(_sceneName);
    if (itr == cameraBuffers_.end()) {
        LOG_WARN("Cannot get transform: Scene is not registered. \n ScenenName : {}", _sceneName);
        return CameraTransform();
    }
    return *(itr->second);
}

CameraTransform CameraManager::GetTransform(Scene* _scene) const {
    if (_scene == nullptr) {
        LOG_WARN("Cannot get transform: Scene is nullptr.");
        return CameraTransform();
    }
    return GetTransform(_scene->GetName());
}
