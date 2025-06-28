#include "CameraActionSystem.h"

/// engine
#define DELTA_TIME
#include "EngineInclude.h"

/// ECS
#include "ECSManager.h"

// component
#include "component/effect/CameraAction.h"
#include "component/transform/CameraTransform.h"

CameraActionSystem::CameraActionSystem()
    : ISystem(SystemType::Effect) {}

CameraActionSystem::~CameraActionSystem() {}

void CameraActionSystem::Initialize() {}
void CameraActionSystem::Finalize() {}

void CameraActionSystem::UpdateEntity(GameEntity* _entity) {
    auto action = getComponent<CameraAction>(_entity);
    auto camera = getComponent<CameraTransform>(_entity);

    if (!action || !camera) {
        return;
    }

    if (!action->isPlaying()) {
        return;
    }

    float currentTime = action->getCurrentTime();
    currentTime -= getMainDeltaTime();
    action->setCurrentTime(currentTime);

    if (currentTime <= 0.f) {
        if (action->isLooping()) {
            action->Play();
        } else {
            action->Stop();
        }
    }

    auto& fovCurve = action->getFovCurve();
    if (!fovCurve.empty()) {
        camera->fovAngleY = CalculateValue::Linear(fovCurve, currentTime);
    }

    auto& aspectCurve = action->getAspectRatioCurve();
    if (!aspectCurve.empty()) {
        camera->aspectRatio = CalculateValue::Linear(aspectCurve, currentTime);
    }

    auto& nearZCurve = action->getNearZCurve();
    if (!nearZCurve.empty()) {
        camera->nearZ = CalculateValue::Linear(nearZCurve, currentTime);
    }

    auto& farZCurve = action->getFarZCurve();
    if (!farZCurve.empty()) {
        camera->nearZ = CalculateValue::Linear(farZCurve, currentTime);
    }

    auto& rotateCurve = action->getRotationCurve();
    if (!rotateCurve.empty()) {
        camera->rotate = CalculateValue::Linear(rotateCurve, currentTime);
    }

    auto& posCurve = action->getPositionCurve();
    if (!posCurve.empty()) {
        camera->translate = CalculateValue::Linear(posCurve, currentTime);
    }

    camera->UpdateMatrix();
}
