#include "ScaleDeformSystem.h"

/// engine
#include "Engine.h"

/// ECS
// component
#include "component/effect/SquashStretchComponent.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

/// math
#include "math/MathEnv.h"
#include <cmath>

using namespace OriGine;

OriGine::ScaleDeformSystem::ScaleDeformSystem() : ISystem(SystemCategory::Effect) {}

void OriGine::ScaleDeformSystem::Initialize() {}
void OriGine::ScaleDeformSystem::Finalize() {}

void OriGine::ScaleDeformSystem::UpdateEntity(EntityHandle _handle) {
    float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    auto* squashStretch = GetComponent<SquashStretchComponent>(_handle);
    if (!squashStretch) {
        return;
    }

    auto* transform = GetComponent<Transform>(_handle);
    if (!transform) {
        return;
    }

    auto* rigidbody = GetComponent<Rigidbody>(_handle);
    if (!rigidbody) {
        return;
    }

    const Vec3f& preVelocity = squashStretch->GetPreVelocity();
    const Vec3f& velocity    = rigidbody->GetVelocity();

    Vec3f frontVec = transform->FrontVector().normalize();
    Vec3f rightVec = Vec3f::Cross(axisY, frontVec).normalize();
    Vec3f upVec    = Vec3f::Cross(frontVec, rightVec).normalize();

    // 前フレームからの速度変化を計算
    // 押しつぶしに使用するため、どれだけ急激に減速したかを知りたい。
    // 減速のみを考慮するため、増速は無視（0にクランプ）する。
    Vec3f deltaV = Vec3f();
    for (int i = 0; i < 3; ++i) {
        if (std::abs(velocity[i]) < std::abs(preVelocity[i])) {
            deltaV[i] = std::abs(preVelocity[i] - velocity[i]);
        }
    }

    // ローカル空間へ変換（正規化しない）
    Vec3f accelLocal = {Vec3f::Dot(deltaV, rightVec), Vec3f::Dot(deltaV, upVec), Vec3f::Dot(deltaV, frontVec)};

    Vec3f velocityLocal = {
        Vec3f::Dot(velocity, rightVec),
        Vec3f::Dot(velocity, upVec),
        Vec3f::Dot(velocity, frontVec)};

    /// ヒステリシス
    // velocity
    float veloThresholdEnter                    = squashStretch->GetVelocityStretchThresholdEnter();
    float veloThresholdExit                     = squashStretch->GetVelocityStretchThresholdExit();
    std::array<bool, 3> hasVelocityImpactByAxis = squashStretch->IsVelocityStretchActiveByAxis();
    // accel
    float accelThresholdEnter                = squashStretch->GetAccelSquashThresholdEnter();
    float accelThresholdExit                 = squashStretch->GetAccelSquashThresholdExit();
    std::array<bool, 3> hasAccelImpactByAxis = squashStretch->IsAccelSquashActiveByAxis();

    // 軸ごとに
    for (int i = 0; i < 3; ++i) {
        float v = velocityLocal[i];

        if (!hasVelocityImpactByAxis[i] && std::abs(v) > veloThresholdEnter) {
            hasVelocityImpactByAxis[i] = true;
        } else if (hasVelocityImpactByAxis[i] && std::abs(v) < veloThresholdExit) {
            hasVelocityImpactByAxis[i] = false;
        }
        squashStretch->SetVelocityStretchActive(AxisIndex(i), hasVelocityImpactByAxis[i]);

        float accel = accelLocal[i];
        // enter確認
        if (!hasAccelImpactByAxis[i] && std::abs(accel) > accelThresholdEnter) {
            hasAccelImpactByAxis[i] = true;
        } else if (hasAccelImpactByAxis[i] && std::abs(accel) < accelThresholdExit) {
            // exit 確認
            hasAccelImpactByAxis[i] = false;
        }
        squashStretch->SetVelocityStretchActive(AxisIndex(i), hasAccelImpactByAxis[i]);
    }

    /// バネによる計算
    float accelInfluence    = squashStretch->GetAccelSquashInfluence();
    float velocityInfluence = squashStretch->GetVelocityStretchInfluence();
    float maxStretch        = squashStretch->GetMaxStretch();
    float maxSquash         = squashStretch->GetMaxSquash();

    // --- Stretch（速度ベース） ---
    Vec3f stretch = velocityLocal * velocityInfluence;

    // --- Squash（加速度ベース） ---
    Vec3f squash = accelLocal * accelInfluence;

    for (int i = 0; i < 3; ++i) {
        // しきい値以下の影響は無効化
        if (!hasVelocityImpactByAxis[i]) {
            stretch[i] = 0.f;
        }

        if (!hasAccelImpactByAxis[i]) {
            squash[i] = 0.f;
        }
    }

    // Clamp
    if (stretch.lengthSq() >= maxStretch * maxStretch) {
        stretch = ClampElement(stretch, 0.f, maxStretch);
    }
    if (squash.lengthSq() >= maxSquash * maxSquash) {
        squash = ClampElement(squash, 0.f, maxSquash);
    }

    // 伸びは +、潰れは − として扱う
    Vec3f targetScale = squashStretch->GetBaseScale() + stretch - squash;

    // 体積を一定に保つための補正
    float volume                  = targetScale[X] * targetScale[Y] * targetScale[Z];
    constexpr float collectionPow = -1.0f / 3.0f; // 立方根
    float correction              = std::pow(volume, collectionPow);
    targetScale *= correction;

    // バネ計算して、もとに戻るように
    Vec3f displacement = targetScale - transform->scale;

    // F = kx
    Vec3f scaleVelo = squashStretch->GetScaleVelocity();
    Vec3f force     = displacement * squashStretch->GetStiffness() - scaleVelo * squashStretch->GetDamping();
    scaleVelo += force * deltaTime;
    transform->scale += scaleVelo * deltaTime;
    // もし計算結果が NaN になってしまった場合は、スケールを基準値にリセットする
    if (std::isnan(transform->scale[X]) || std::isnan(transform->scale[Y]) || std::isnan(transform->scale[Z])) {
        transform->scale = squashStretch->GetBaseScale();
        scaleVelo        = Vec3f(0, 0, 0);
    }

    // 処理終了後に次フレームのために速度を保存
    squashStretch->SetPreVelocity(rigidbody->GetVelocity());
    squashStretch->SetScaleVelocity(scaleVelo);
}
