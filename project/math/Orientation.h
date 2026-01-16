#pragma once

/// math
#include <Matrix4x4.h>
#include <Quaternion.h>
#include <Vector3.h>

namespace OriGine {

/// <summary>
/// オリエンテーション情報
/// </summary>
struct Orientation {
    Vec3f axis[3];
    Quaternion rot;

    static Orientation Identity();

    /// <summary>
    /// 回転を設定
    /// </summary>
    /// <param name="_q">回転クォータニオン</param>
    void SetRotation(const Quaternion& _q) {
        rot = _q.normalize();
        UpdateAxes();
    }

    /// <summary>
    /// 指定の回転を加算
    /// </summary>
    /// <param name="_q">回転クォータニオン</param>
    void Rotate(const Quaternion& _q) {
        rot = (_q * rot).normalize();
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
