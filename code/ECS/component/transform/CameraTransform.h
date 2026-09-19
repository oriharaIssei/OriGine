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

namespace OriGine {

/// <summary>
/// CameraTransform コンポーネント
/// </summary>
class CameraTransform
    : public IComponent {
public:
    ORIGINE_COMPONENT();

    CameraTransform() {}
    ~CameraTransform() {}

    void Initialize(Scene* _scene, const EntityHandle& _entity);

    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    void Finalize();

    void UpdateMatrix();

public:
    ORIGINE_FIELD(no_save); // メインカメラとして使うかはエディタ/実行時の選択であり保存しない
    bool canUseMainCamera = true;
    Quaternion rotate = Quaternion();
    Vec3f translate   = {0.0f, 0.0f, 0.0f};
    ORIGINE_FIELD(no_save); // UpdateMatrix() で再計算されるため保存しない
    Matrix4x4 viewMat = MakeMatrix4x4::Identity();

    // 垂直方向視野角
    float fovAngleY = 45.0f * 3.141592654f / 180.0f;
    // ビューポートのアスペクト比
    float aspectRatio = (float)16 / 9;
    // 深度限界（手前側）
    float nearZ = 0.1f;
    // 深度限界（奥側）
    float farZ = 1000.0f;
    ORIGINE_FIELD(no_save); // UpdateMatrix() で再計算されるため保存しない
    Matrix4x4 projectionMat = MakeMatrix4x4::Identity();

public:
    struct ConstantBuffer {
        Vec3f cameraPos;
        float padding;
        Matrix4x4 view; // ワールド → ビュー変換行列
        Matrix4x4 viewTranspose;
        Matrix4x4 projection; // ビュー → プロジェクション変換行列
        ConstantBuffer& operator=(const CameraTransform& _comp) {
            cameraPos     = _comp.viewMat[3];
            view          = _comp.viewMat;
            viewTranspose = _comp.viewMat.transpose();
            projection    = _comp.projectionMat;
            return *this;
        }
    };
};

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする。
// 手書きの to_json/from_json は D-4 で削除済み(ComponentArray 経由以外の呼び出しが
// 無いことを確認済み。docs/todo.html Phase 3 D-4 参照)。
template <>
inline constexpr bool kUsesDescriptorSerialization<CameraTransform> = true;

} // namespace OriGine
