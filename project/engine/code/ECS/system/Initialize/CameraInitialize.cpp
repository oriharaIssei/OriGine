#include "CameraInitialize.h"

/// manager
#include "camera/CameraManager.h"
/// components
#include "component/transform/CameraTransform.h"

CameraInitialize::CameraInitialize() : ISystem(SystemCategory::Initialize) {}
CameraInitialize::~CameraInitialize() {}

void CameraInitialize::Initialize() {}
void CameraInitialize::Finalize() {}

void CameraInitialize::UpdateEntity(Entity* _entity) {
    CameraTransform* cameraTransform = getComponent<CameraTransform>(_entity);
    if (!cameraTransform) {
        return;
    }

    cameraTransform->UpdateMatrix();
    CameraManager::getInstance()->setTransform(*cameraTransform);
    CameraManager::getInstance()->DataConvertToBuffer();
}
