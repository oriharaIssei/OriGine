#pragma once

/// stl
#include <deque>

/// math
#include "Vector3.h"
#include <cfloat>
#include <limits>

namespace OriGine {

/// <summary>
/// 4点間のCatmull-Romスプライン補間
/// </summary>
/// <param name="_p0">制御点0</param>
/// <param name="_p1">開始点</param>
/// <param name="_p2">終了点</param>
/// <param name="_p3">制御点3</param>
/// <param name="_t">補間係数(0-1)</param>
/// <returns>補間後の座標</returns>
Vec3f CatmullRomSpline(const Vec3f& _p0, const Vec3f& _p1, const Vec3f& _p2, const Vec3f& _p3, float _t);

/// <summary>
/// 複数の制御点を通るCatmull-Romスプライン曲線を生成
/// </summary>
/// <param name="_points">制御点のリスト</param>
/// <param name="_samplePerSegment">セグメントあたりの分割数</param>
/// <returns>生成された点群</returns>
std::deque<Vec3f> CatmullRomSpline(const std::deque<Vec3f>& _points, int _samplePerSegment);

/// <summary>
/// スプライン曲線の全長を計算
/// </summary>
/// <param name="_points"></param>
/// <returns></returns>
float CalcSplineLength(const std::deque<Vec3f>& _points);

/// <summary>
/// 指定位置に最も近いスプラインセグメントのインデックスを計算
/// </summary>
/// <param name="_points">制御点</param>
/// <param name="_position">指定座標</param>
/// <returns></returns>
std::pair<uint32_t, uint32_t> CalcPointSegmentIndex(const std::deque<Vec3f>& _points, const Vec3f& _position);

/// <summary>
/// 指定された始点からの距離が収まるスプラインセグメントのインデックスを計算
/// </summary>
/// <param name="_points">制御点</param>
/// <param name="_distance">始点からの現在の距離</param>
/// <returns></returns>
std::pair<uint32_t, uint32_t> CalcDistanceSegmentIndex(const std::deque<Vec3f>& _points, float _distance);

/// <summary>
/// 指定された始点からの距離に対応するスプライン上の点を計算
/// </summary>
/// <param name="_points"></param>
/// <param name="_distance"></param>
/// <returns></returns>
Vec3f CalcPointOnSplineByDistance(const std::deque<Vec3f>& _points, float _distance);

} // namespace OriGine
