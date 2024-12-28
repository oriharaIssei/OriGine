#pragma once

#include <functional>
#include <string>
#include <vector>

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
} // namespace ImGui
