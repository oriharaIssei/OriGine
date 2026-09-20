#pragma once

/// math
#include <Matrix4x4.h>
#include <Quaternion.h>
#include <Vector3.h>

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

/// <summary>
/// オリエンテーション情報
/// </summary>
struct ORIGINE_API Orientation {
    Vec3f axis[3];
    Quaternion rot;

    static Orientation Identity();

    /// <summary>
    /// 回転を設定
    /// </summary>
    /// <param name="q">回転クォータニオン</param>
    void SetRotation(const Quaternion& q) {
        rot = q.normalize();
        UpdateAxes();
    }

    /// <summary>
    /// 指定の回転を加算
    /// </summary>
    /// <param name="q">回転クォータニオン</param>
    void Rotate(const Quaternion& q) {
        rot = (q * rot).normalize();
        UpdateAxes();
    }

    /// <summary>
    /// Quaternionから Axisを更新
    /// </summary>
    void UpdateAxes();

    /// <summary>
    /// 右方向ベクトルを取得
    /// </summary>
    const Vec3f& Right() const { return axis[0]; }
    /// <summary>
    /// 上方向ベクトルを取得
    /// </summary>
    const Vec3f& Up() const { return axis[1]; }
    /// <summary>
    /// 前方向ベクトルを取得
    /// </summary>
    const Vec3f& Forward() const { return axis[2]; }
};

} // namespace OriGine
