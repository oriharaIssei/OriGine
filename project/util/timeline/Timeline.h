#pragma once

/// stl
// pointer
#include <functional>
// container
#include <vector>
// string
#include "component/animation/ModelNodeAnimation.h"

#include <string>

namespace ImGui {
/// <summary>
/// タイムラインボタン
/// </summary>
/// <param name="_label">ラベル</param>
/// <param name="_nodeTimes">ノードの時間</param>
/// <param name="_duration">アニメーションの時間</param>
/// <param name="_updateOnNodeDragged">ノードがドラッグされたときの更新</param>
/// <param name="_sliderPopupUpdate">スライダーポップアップの更新</param>
/// <param name="_nodePopupUpdate">ノードポップアップの更新</param>
/// <returns>更新されたか</returns>
bool TimeLineButtons(
    const std::string& _label,
    std::vector<float>& _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged = nullptr,
    std::function<void(float _currentTime)> _sliderPopupUpdate  = nullptr,
    std::function<bool(int nodeIndex)> _nodePopupUpdate         = nullptr);

bool TimeLineButtons(
    const std::string& _label,
    AnimationCurve<int>& _keyFrames,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged = nullptr,
    std::function<void(float _currentTime)> _sliderPopupUpdate  = nullptr,
    std::function<bool(int nodeIndex)> _nodePopupUpdate         = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
/// <returns></returns>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<float>& _keyFrames,
    float _duration,
    float _defaultValue                   = 0.0f,
    std::function<void(int)> _howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec2f>& _keyFrames,
    float _duration,
    const Vec2f& _defaultValue            = Vec2f(0.0f, 0.0f),
    std::function<void(int)> _howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec3f>& _keyFrames,
    float _duration,
    const Vec3f& _defaultValue            = Vec3f(0.0f, 0.0f, 0.0f),
    std::function<void(int)> _howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration,
    const Vec4f& _defaultValue            = Vec4f(0.0f, 0.0f, 0.0f, 0.0f),
    std::function<void(int)> _howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Quaternion>& _keyFrames,
    float _duration,
    const Quaternion& _defaultValue       = Quaternion(0.0f, 0.0f, 0.0f, 1.0f),
    std::function<void(int)> _howEditItem = nullptr);

/// <summary>
/// animationCurve(Color)をKeyFrameエディタで編集する
/// </summary>
/// <param name="_label"></param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">アニメーションの全体時間</param>
/// <param name="_defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="_howEditItem">値を編集するための関数</param>
bool EditColorKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration,
    const Vec4f& _defaultValue            = Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
    std::function<void(int)> _howEditItem = nullptr);

} // namespace ImGui
