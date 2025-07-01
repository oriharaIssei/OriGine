#pragma once

/// <summary>
/// Radian を Degree に変換
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
float radianToDegree(float radian) {
    return radian * (180.0f / 3.14159265358979323846f);
}
/// <summary>
/// Degree を Radian に変換
/// </summary>
float degreeToRadian(float degree) {
    return degree * (3.14159265358979323846f / 180.0f);
}
