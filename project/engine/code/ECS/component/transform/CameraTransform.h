#pragma once

/// microsoft
#include <wrl.h>
/// directX12
#include <d3d12.h>

/// engine
#include "component/IComponent.h"

/// math
#include <Matrix4x4.h>
#include <Quaternion.h>
#include <Vector3.h>

class CameraTransform
    : public IComponent {
    friend void to_json(nlohmann::json& j, const CameraTransform& r);
    friend void from_json(const nlohmann::json& j, CameraTransform& r);

public:
    CameraTransform() {}
    ~CameraTransform() {}

    void Initialize(GameEntity* _hostEntity = nullptr);

    bool Edit() override;

    void Finalize() override;

    void UpdateMatrix();
    // Vec3f scale;
    Quaternion rotate = Quaternion();
    Vec3f translate   = {0.0f, 0.0f, 0.0f};
    Matrix4x4 viewMat;

    // 垂直方向視野角
    float fovAngleY = 45.0f * 3.141592654f / 180.0f;
    // ビューポートのアスペクト比
    float aspectRatio = (float)16 / 9;
    // 深度限界（手前側）
    float nearZ = 0.1f;
    // 深度限界（奥側）
    float farZ = 1000.0f;
    Matrix4x4 projectionMat;

public:
    struct ConstantBuffer {
        Vec3f cameraPos;
        float padding;
        Matrix4x4 view; // ワールド → ビュー変換行列
        Matrix4x4 viewTranspose;
        Matrix4x4 projection; // ビュー → プロジェクション変換行列
        ConstantBuffer& operator=(const CameraTransform& camera) {
            cameraPos     = camera.viewMat[3];
            view          = camera.viewMat;
            viewTranspose = camera.viewMat.transpose();
            projection    = camera.projectionMat;
            return *this;
        }
    };
};
