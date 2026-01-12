#pragma once

/// stl
#include <deque>

/// math
#include "Vector3.h"

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
} // namespace OriGine
