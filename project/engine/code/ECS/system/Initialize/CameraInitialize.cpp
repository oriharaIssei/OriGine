#include "CameraInitialize.h"

/// manager
#include "camera/CameraManager.h"
/// components
#include "component/transform/CameraTransform.h"

using namespace OriGine;

CameraInitialize::CameraInitialize() : ISystem(SystemCategory::Initialize) {}
CameraInitialize::~CameraInitialize() {}

void CameraInitialize::Initialize() {}
void CameraInitialize::Finalize() {}

void CameraInitialize::UpdateEntity(Entity* _entity) {
    CameraTransform* cameraTransform = GetComponent<CameraTransform>(_entity);
    if (!cameraTransform) {
        return;
    }

    cameraTransform->UpdateMatrix();
    CameraManager::GetInstance()->SetTransform(*cameraTransform);
    CameraManager::GetInstance()->DataConvertToBuffer();
}
