#pragma once

///stl
//pointer
#include <functional>
//container
#include <vector>
//string
#include "animation/Animation.h"

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
    std::vector<float*> _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged = nullptr,
    std::function<void(float _currentTime)> _sliderPopupUpdate  = nullptr,
    std::function<void(int nodeIndex)> _nodePopupUpdate         = nullptr);

/// <summary>
/// AnimationCurve<>を編集
/// </summary>
/// <param name="_label">ラベル</param>
/// <param name="_keyFrames">編集対象</param>
/// <param name="_duration">総合時間</param>
/// <returns></returns>
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<float>& _keyFrames,
    float _duration);
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec3f>& _keyFrames,
    float _duration);
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration);
bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Quaternion>& _keyFrames,
    float _duration);
} // namespace ImGui
