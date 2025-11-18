#include "CameraActionSystem.h"

/// engine
#define DELTA_TIME
#include "EngineInclude.h"

// component
#include "component/effect/CameraAction.h"
#include "component/transform/CameraTransform.h"

CameraActionSystem::CameraActionSystem()
    : ISystem(SystemCategory::Effect) {}

CameraActionSystem::~CameraActionSystem() {}

void CameraActionSystem::Initialize() {}
void CameraActionSystem::Finalize() {}

void CameraActionSystem::UpdateEntity(Entity* _entity) {
    auto action = GetComponent<CameraAction>(_entity);
    auto camera = GetComponent<CameraTransform>(_entity);

    // コンポーネントが存在しない場合は処理を抜ける
    if (!action || !camera) {
        return;
    }

    // 再生中でなければ処理を抜ける
    if (!action->isPlaying()) {
        return;
    }

    // アニメーション時間を進める
    float currentTime = action->GetCurrentTime();
    currentTime -= GetMainDeltaTime();
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
        camera->nearZ = CalculateValue::Linear(farZCurve, currentTime);
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
}
