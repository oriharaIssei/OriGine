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
/// <param name="p0">制御点0</param>
/// <param name="p1">開始点</param>
/// <param name="p2">終了点</param>
/// <param name="p3">制御点3</param>
/// <param name="t">補間係数(0-1)</param>
/// <returns>補間後の座標</returns>
Vec3f CatmullRomSpline(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, float t);

/// <summary>
/// 複数の制御点を通るCatmull-Romスプライン曲線を生成
/// </summary>
/// <param name="points">制御点のリスト</param>
/// <param name="samplePerSegment">セグメントあたりの分割数</param>
/// <returns>生成された点群</returns>
std::deque<Vec3f> CatmullRomSpline(const std::deque<Vec3f>& points, int samplePerSegment);

/// <summary>
/// スプライン曲線の全長を計算
/// </summary>
/// <param name="points"></param>
/// <returns></returns>
float CalcSplineLength(const std::deque<Vec3f>& points);

/// <summary>
/// 指定位置に最も近いスプラインセグメントのインデックスを計算
/// </summary>
/// <param name="points">制御点</param>
/// <param name="position">指定座標</param>
/// <returns></returns>
std::pair<uint32_t, uint32_t> CalcPointSegmentIndex(const std::deque<Vec3f>& points, const Vec3f& position);

/// <summary>
/// 指定された始点からの距離が収まるスプラインセグメントのインデックスを計算
/// </summary>
/// <param name="points">制御点</param>
/// <param name="distance">始点からの現在の距離</param>
/// <returns></returns>
std::pair<uint32_t, uint32_t> CalcDistanceSegmentIndex(const std::deque<Vec3f>& points, float distance);

/// <summary>
/// 指定位置までのスプライン上の距離を計算
/// </summary>
/// <param name="points"></param>
/// <param name="position"></param>
/// <returns></returns>
float CalcDistanceAlongSpline(const std::deque<Vec3f>& points, const Vec3f& position);

/// <summary>
/// 指定された始点からの距離に対応するスプライン上の点を計算
/// </summary>
/// <param name="points"></param>
/// <param name="distance"></param>
/// <returns></returns>
Vec3f CalcPointOnSplineByDistance(const std::deque<Vec3f>& points, float distance);

} // namespace OriGine
