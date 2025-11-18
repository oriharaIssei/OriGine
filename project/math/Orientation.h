#pragma once

#include <Matrix4x4.h>
#include <Quaternion.h>
#include <Vector3.h>

/// <summary>
/// オリエンテーション情報
/// </summary>
struct Orientation {
    Vec3f axis[3];
    Quaternion rot;

    static Orientation Identity();

    void SetRotation(const Quaternion& q) {
        rot = q.normalize();
        UpdateAxes();
    }

    void Rotate(const Quaternion& q) {
        rot = (q * rot).normalize();
        UpdateAxes();
    }

    /// <summary>
    /// Quaternionから Axisを更新
    /// </summary>
    void UpdateAxes();

    const Vec3f& Right() const { return axis[0]; }
    const Vec3f& Up() const { return axis[1]; }
    const Vec3f& Forward() const { return axis[2]; }
};
