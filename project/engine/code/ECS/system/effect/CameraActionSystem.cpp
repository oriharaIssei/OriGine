#include "CameraActionSystem.h"

/// engine
#include "Engine.h"

#include "camera/CameraManager.h"
// component
#include "component/effect/CameraAction.h"
#include "component/transform/CameraTransform.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
CameraActionSystem::CameraActionSystem()
    : ISystem(SystemCategory::Effect) {}

/// <summary>
/// デストラクタ
/// </summary>
CameraActionSystem::~CameraActionSystem() {}

/// <summary>
/// 初期化処理
/// </summary>
void CameraActionSystem::Initialize() {
}

/// <summary>
/// 終了処理
/// </summary>
void CameraActionSystem::Finalize() {}

/// <summary>
/// 各エンティティのカメラアクションを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void CameraActionSystem::UpdateEntity(EntityHandle _handle) {
    auto action = GetComponent<CameraAction>(_handle);
    auto camera = GetComponent<CameraTransform>(_handle);

    // コンポーネントが存在しない場合は処理を抜ける
    if (!action || !camera) {
        return;
    }

    // 再生中でなければ処理を抜ける
    if (!action->isPlaying()) {
        return;
    }

    float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    // アニメーション時間を進める
    float currentTime = action->GetTime();
    currentTime -= deltaTime;
    action->SetCurrentTime(currentTime);

    // アニメーション終了判定
    // ループ設定されている場合は最初から再生
    if (currentTime <= 0.f) {
        if (action->isLooping()) {
            action->Play();
        } else {
            action->Stop();
        }
    }

    auto& fovCurve = action->GetFovCurve();
    if (!fovCurve.empty()) {
        camera->fovAngleY = CalculateValue::Linear(fovCurve, currentTime);
    }

    auto& aspectCurve = action->GetAspectRatioCurve();
    if (!aspectCurve.empty()) {
        camera->aspectRatio = CalculateValue::Linear(aspectCurve, currentTime);
    }

    auto& nearZCurve = action->GetNearZCurve();
    if (!nearZCurve.empty()) {
        camera->nearZ = CalculateValue::Linear(nearZCurve, currentTime);
    }

    auto& farZCurve = action->GetFarZCurve();
    if (!farZCurve.empty()) {
        camera->farZ = CalculateValue::Linear(farZCurve, currentTime);
    }

    auto& rotateCurve = action->GetRotationCurve();
    if (!rotateCurve.empty()) {
        camera->rotate = CalculateValue::Linear(rotateCurve, currentTime);
    }

    auto& posCurve = action->GetPositionCurve();
    if (!posCurve.empty()) {
        camera->translate = CalculateValue::Linear(posCurve, currentTime);
    }

    camera->UpdateMatrix();
    CameraManager::GetInstance()->SetTransform(GetScene(), *camera);
    CameraManager::GetInstance()->DataConvertToBuffer(GetScene());
}
