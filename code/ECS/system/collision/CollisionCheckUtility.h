#pragma once

/// math
#include "math/bounds/AABB.h"
#include "math/bounds/OBB.h"
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

/// <summary>
/// 線分上の最近接点を求める（パラメータt: 0～1にクランプ）
/// </summary>
inline Vec3f ClosestPointOnSegment(const Vec3f& _point, const Vec3f& _segStart, const Vec3f& _segEnd) {
    Vec3f ab      = _segEnd - _segStart;
    float abLenSq = ab.lengthSq();
    // 始点と終点が同じ(線分が点に縮退している)場合、下の除算がゼロ除算になる。
    // 縮退した線分上の最近接点は始点そのものなので、それを返す
    if (abLenSq < kEpsilon) {
        return _segStart;
    }
    // 点を線分の延長線上へ射影し、線分上の位置を0〜1の比率tとして得る。
    // t = (point-a)・(b-a) / |b-a|^2
    float t = Vec3f(_point - _segStart).dot(ab) / abLenSq;
    // clampにより、射影が線分の外に出た場合は最寄りの端点に丸められる。
    // これが無いと線分ではなく無限直線に対する最近接点になってしまう
    t       = std::clamp(t, 0.f, 1.f);
    return _segStart + ab * t;
}

/// <summary>
/// 2つの線分間の最近接点ペアを求める
/// </summary>
/// <remarks>
/// カプセル同士の衝突判定の土台になる処理。
/// 各線分上の点を S(s) = p1 + d1*s, T(t) = p2 + d2*t (s,t は 0〜1)と表すと、
/// 2点間の距離の2乗は s と t の2次式になる。これを最小化する s,t は
/// 偏微分が0になる点として連立方程式で解ける。
///
/// ただし解が0〜1の外に出る場合、それは線分ではなく延長線上の答えなので、
/// 範囲内に収めたうえで残る変数を解き直す必要がある(下のtによる場合分け)。
/// 線分が点に縮退しているケースは除算が破綻するため個別に処理する。
///
/// 変数はこのアルゴリズムの慣例的な名前で、それぞれ次を表す:
///   a = |d1|^2 (線分1の長さの2乗) / e = |d2|^2 (線分2の長さの2乗)
///   b = d1・d2 (2線分の向きの一致度) / c = d1・r / f = d2・r
///   r = p1 - p2 (始点同士のずれ)
/// </remarks>
inline void ClosestPointsBetweenSegments(
    const Vec3f& _p1, const Vec3f& _q1, // 線分1
    const Vec3f& _p2, const Vec3f& _q2, // 線分2
    Vec3f& _closest1, Vec3f& _closest2) {

    Vec3f d1 = _q1 - _p1;
    Vec3f d2 = _q2 - _p2;
    Vec3f r  = _p1 - _p2;

    float a = d1.dot(d1);
    float e = d2.dot(d2);
    float f = d2.dot(r);

    float s, t;

    if (a < kEpsilon && e < kEpsilon) {
        // 両方の線分が点に縮退
        _closest1 = _p1;
        _closest2 = _p2;
        return;
    }

    if (a < kEpsilon) {
        // 線分1が点に縮退
        s = 0.f;
        t = std::clamp(f / e, 0.f, 1.f);
    } else {
        float c = d1.dot(r);
        if (e < kEpsilon) {
            // 線分2が点に縮退
            t = 0.f;
            s = std::clamp(-c / a, 0.f, 1.f);
        } else {
            // 一般ケース
            float b     = d1.dot(d2);
            // denomは連立方程式の行列式にあたる。2線分が平行だと a*e == b*b となり0になる
            // (平行な線分では最近接点が一意に定まらないため、方程式が解けない)
            float denom = a * e - b * b;

            if (denom != 0.f) {
                s = std::clamp((b * f - c * e) / denom, 0.f, 1.f);
            } else {
                // 平行な場合はどこを取っても距離が同じなので、線分1の始点(s=0)を代表にする
                s = 0.f;
            }

            // sが確定したら、そのsに対して最も近い線分2上の位置tを求める
            t = (b * s + f) / e;

            // tが0〜1を外れた＝最近接点が線分2の外にある。
            // その場合は線分2の端点(t=0または1)に固定し、その端点に最も近い
            // 線分1上の位置としてsを計算し直す。
            // sだけをclampして済ませると端点付近で誤った距離になるため、この解き直しが必要
            if (t < 0.f) {
                t = 0.f;
                s = std::clamp(-c / a, 0.f, 1.f);
            } else if (t > 1.f) {
                t = 1.f;
                s = std::clamp((b - c) / a, 0.f, 1.f);
            }
        }
    }

    _closest1 = _p1 + d1 * s;
    _closest2 = _p2 + d2 * t;
}

/// <summary>
/// AABBと点の最近接点を求める
/// </summary>
inline Vec3f ClosestPointOnAABB(const Vec3f& _point, const Bounds::AABB& _aabb) {
    // AABBは軸に平行なので、各軸を独立に扱える。
    // 軸ごとに点の座標を[min,max]へclampするだけで最近接点になる
    // (点が箱の内側にある場合はclampが効かず、点自身がそのまま返る)
    Vec3f aabbMin = _aabb.Min();
    Vec3f aabbMax = _aabb.Max();
    return {
        std::clamp(_point[X], aabbMin[X], aabbMax[X]),
        std::clamp(_point[Y], aabbMin[Y], aabbMax[Y]),
        std::clamp(_point[Z], aabbMin[Z], aabbMax[Z])};
}

/// <summary>
/// OBBと点の最近接点を求める
/// </summary>
/// <remarks>
/// OBBは傾いているためワールド軸でのclampが使えない。
/// 代わりに中心からの相対位置をOBB自身の3軸に射影し、各軸方向の距離を半径(halfSize)で
/// clampしてから足し戻すことで、AABBと同じ考え方をOBBのローカル軸上で行う。
/// 実質的に「OBBのローカル空間へ移してclampし、ワールドへ戻す」操作と同じだが、
/// 行列を組まず内積だけで済ませている。
/// </remarks>
inline Vec3f ClosestPointOnOBB(const Vec3f& _point, const Bounds::OBB& _obb) {
    Vec3f d      = _point - _obb.center_;
    Vec3f result = _obb.center_;

    for (int i = 0; i < 3; ++i) {
        // 内積でこの軸方向に中心からどれだけ離れているかを取り出す
        float dist = d.dot(_obb.orientations_.axis[i]);
        // 箱の外へはみ出した分を面上に丸める
        dist       = std::clamp(dist, -_obb.halfSize_[i], _obb.halfSize_[i]);
        result     = result + _obb.orientations_.axis[i] * dist;
    }

    return result;
}

} // namespace OriGine
