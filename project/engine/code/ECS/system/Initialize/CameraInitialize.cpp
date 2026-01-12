#include "CameraInitialize.h"

/// manager
#include "camera/CameraManager.h"
/// components
#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"

using namespace OriGine;

CameraInitialize::CameraInitialize() : ISystem(SystemCategory::Initialize) {}
CameraInitialize::~CameraInitialize() {}

/// <summary>
/// 初期化
/// </summary>
void CameraInitialize::Initialize() {}

/// <summary>
/// 終了処理
/// </summary>
void CameraInitialize::Finalize() {}

/// <summary>
/// エンティティの更新（カメラの初期化）
/// </summary>
/// <param name="_handle">エンティティハンドル</param>
void CameraInitialize::UpdateEntity(EntityHandle _handle) {
    CameraTransform* cameraTransform = GetComponent<CameraTransform>(_handle);
    if (!cameraTransform) {
        return;
    }

    // transformがあれば、同期
    Transform* transform = GetComponent<Transform>(_handle);
    if (transform) {
        transform->rotate    = cameraTransform->rotate;
        transform->translate = cameraTransform->translate;
        transform->UpdateMatrix();
    }

    cameraTransform->UpdateMatrix();
    CameraManager::GetInstance()->SetTransform(GetScene(), *cameraTransform);
    CameraManager::GetInstance()->DataConvertToBuffer(GetScene());
}
