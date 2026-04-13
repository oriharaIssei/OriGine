#pragma once

/// stl
// pointer
#include <functional>
// container
#include <vector>
// string

#include <string>
#include <component/animation/AnimationData.h>

namespace ImGui {
/// <summary>
/// タイムラインボタン
/// </summary>
/// <param name="label">ラベル</param>
/// <param name="nodeTimes">ノードの時間</param>
/// <param name="duration">アニメーションの時間</param>
/// <param name="updateOnNodeDragged">ノードがドラッグされたときの更新</param>
/// <param name="sliderPopupUpdate">スライダーポップアップの更新</param>
/// <param name="nodePopupUpdate">ノードポップアップの更新</param>
/// <returns>更新されたか</returns>
bool TimeLineButtons(
    const std::string& label,
    std::vector<float>& nodeTimes,
    float duration,
    std::function<void(float newNodeTime)> updateOnNodeDragged = nullptr,
    std::function<void(float currentTime)> sliderPopupUpdate  = nullptr,
    std::function<bool(int nodeIndex)> nodePopupUpdate         = nullptr);

bool TimeLineButtons(
    const std::string& label,
    ::OriGine::AnimationCurve<int>& keyFrames,
    float duration,
    std::function<void(float newNodeTime)> updateOnNodeDragged = nullptr,
    std::function<void(float currentTime)> sliderPopupUpdate  = nullptr,
    std::function<bool(int nodeIndex)> nodePopupUpdate         = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
/// <returns></returns>
bool EditKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<float>& keyFrames,
    float duration,
    float defaultValue                   = 0.0f,
    std::function<void(int)> howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<::OriGine::Vec2f>& keyFrames,
    float duration,
    const ::OriGine::Vec2f& defaultValue            = ::OriGine::Vec2f(0.0f, 0.0f),
    std::function<void(int)> howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<::OriGine::Vec3f>& keyFrames,
    float duration,
    const ::OriGine::Vec3f& defaultValue            = ::OriGine::Vec3f(0.0f, 0.0f, 0.0f),
    std::function<void(int)> howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<::OriGine::Vec4f>& keyFrames,
    float duration,
    const ::OriGine::Vec4f& defaultValue            = ::OriGine::Vec4f(0.0f, 0.0f, 0.0f, 0.0f),
    std::function<void(int)> howEditItem = nullptr);

/// <summary>
/// animationCurveをKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
bool EditKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<::OriGine::Quaternion>& keyFrames,
    float duration,
    const ::OriGine::Quaternion& defaultValue       = ::OriGine::Quaternion(0.0f, 0.0f, 0.0f, 1.0f),
    std::function<void(int)> howEditItem = nullptr);

/// <summary>
/// animationCurve(Color)をKeyFrameエディタで編集する
/// </summary>
/// <param name="label"></param>
/// <param name="keyFrames">編集対象</param>
/// <param name="duration">アニメーションの全体時間</param>
/// <param name="defaultValue">nodeを初期化する際に使用する値</param>
/// <param name="howEditItem">値を編集するための関数</param>
bool EditColorKeyFrame(
    const std::string& label,
    ::OriGine::AnimationCurve<::OriGine::Vec4f>& keyFrames,
    float duration,
    const ::OriGine::Vec4f& defaultValue            = ::OriGine::Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
    std::function<void(int)> howEditItem = nullptr);

} // namespace ImGui
