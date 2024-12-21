#pragma once

#include <functional>
#include <string>
#include <vector>

namespace ImGui {
bool TimeLineButtons(
    const std::string& _label,
    std::vector<float*> _nodeTimes,
    float& _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged     = nullptr,
    std::function<void()> _sliderPopupUpdate                        = nullptr,
    std::function<void(int nodeIndex)> _nodePopupUpdate             = nullptr,
    std::function<void(float newDuration)> _updateOnDurationChanged = nullptr);
} // namespace ImGui
