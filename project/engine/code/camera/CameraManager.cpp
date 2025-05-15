#include "CameraManager.h"

#include "debugCamera/DebugCamera.h"
#include "Engine.h"

/// math
#include "Matrix4x4.h"
#include "Vector2.h"

CameraManager::CameraManager()
    : IModule() {}
CameraManager::~CameraManager() {}

Vec2f CameraManager::world2Screen(const Matrix4x4& _worldMat) const {
    // ワールド座標を取得（平行移動成分）
    Vec3f worldPos(_worldMat[3][0], _worldMat[3][1], _worldMat[3][2]);
    Vec4f pos4(worldPos, 1.0f);

    // 合成行列（ワールド→ビュー→プロジェクション→ビューポート）
    const auto& viewMat = cTransform_->viewMat;
    const auto& projMat = cTransform_->projectionMat;
    Matrix4x4 vpvpMat    = viewMat * projMat * viewPortMat_;

    // 変換
    Vec4f screenPos = pos4 * vpvpMat;

    // 透視除算
    if (screenPos[W] == 0.0f) {
        return Vec2f(0.0f, 0.0f);
    }
    return Vec2f(screenPos[X] / screenPos[W], screenPos[Y] / screenPos[W]);
}

CameraManager* CameraManager::getInstance() {
    static CameraManager instance{};
    return &instance;
}

void CameraManager::Initialize() {
    cTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    cTransform_->Initialize();
    cTransform_->UpdateMatrix();
    cTransform_.ConvertToBuffer();

    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize();

    const Vec2f& windowSize = Engine::getInstance()->getWinApp()->getWindowSize();
    viewPortMat_            = MakeMatrix::ViewPort(0, 0, windowSize[X], windowSize[Y], 0.0f, 1.0f);
}

void CameraManager::UpdateMatrix() {
    const Vec2f& windowSize = Engine::getInstance()->getWinApp()->getWindowSize();
    viewPortMat_            = MakeMatrix::ViewPort(0.0f, 0.0f, windowSize[X], windowSize[Y], 0.f, 1000.f);
}

void CameraManager::DebugUpdate() {
    debugCamera_->Update();

    // debugCameraUpdate で 更新された情報を 取得
    setTransform(debugCamera_->getCameraTransform());

    // 情報を Buffuer に 渡す
    cTransform_.ConvertToBuffer();
}

void CameraManager::Finalize() {
    cTransform_.Finalize();
}
