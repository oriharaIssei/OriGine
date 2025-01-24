#pragma once

//math
#include "Quaternion.h"
#include "Vector3.h"

/// <summary>
/// 3次スプライン補間を行う関数
/// </summary>
/// <param name="previousPoint">前のポイント</param>
/// <param name="previousTangent">前の接線</param>
/// <param name="nextPoint">次のポイント</param>
/// <param name="nextTangent">次の接線</param>
/// <param name="interpolationValue">補間値 (0.0f から 1.0f の範囲)</param>
/// <returns>補間された Vec3f の値</returns>
Vec3f CubicSpline(const Vec3f& previousPoint, const Vec3f& previousTangent, const Vec3f& nextPoint, const Vec3f& nextTangent, float interpolationValue);

/// <summary>
/// 3次スプライン補間を行う関数
/// </summary>
/// <param name="previousPoint">前のポイント</param>
/// <param name="previousTangent">前の接線</param>
/// <param name="nextPoint">次のポイント</param>
/// <param name="nextTangent">次の接線</param>
/// <param name="interpolationValue">補間値 (0.0f から 1.0f の範囲)</param>
/// <returns>補間された Quaternion の値</returns>
Quaternion CubicSpline(const Quaternion& previousPoint, const Quaternion& previousTangent, const Quaternion& nextPoint, const Quaternion& nextTangent, float interpolationValue);
