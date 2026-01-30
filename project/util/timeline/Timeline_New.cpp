#include "Timeline.h"

#ifdef _DEBUG

/// stl
#include <algorithm>

/// timeline modules
#include "TimelineConfig.h"
#include "TimelineCore.h"
#include "TimelinePopup.h"
#include "TimelineEditor.h"
#include "TimelineButtons.h"

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "myGui/MyGui.h"

namespace ImGui {

//==============================================================================
// TimeLineButtons 実装
//==============================================================================

bool TimeLineButtons(
    const std::string& _label,
    std::vector<float>& _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void(float _currentTime)> _sliderPopupUpdate,
    std::function<bool(int nodeIndex)> _nodePopupUpdate) {
    return TimeLineButtonsImpl(
        _label,
        _nodeTimes,
        _duration,
        [](const float& time) { return time; },
        [](float& time, float newTime) { time = newTime; },
        _updateOnNodeDragged,
        _sliderPopupUpdate,
        _nodePopupUpdate);
}

bool TimeLineButtons(
    const std::string& _label,
    OriGine::AnimationCurve<int>& _keyFrames,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void(float _currentTime)> _sliderPopupUpdate,
    std::function<bool(int nodeIndex)> _nodePopupUpdate) {
    return TimeLineButtonsImpl(
        _label,
        _keyFrames,
        _duration,
        [](const OriGine::KeyFrame<int>& kf) { return kf.time; },
        [](OriGine::KeyFrame<int>& kf, float newTime) { kf.time = newTime; },
        _updateOnNodeDragged,
        _sliderPopupUpdate,
        _nodePopupUpdate);
}

//==============================================================================
// EditKeyFrame 実装
//==============================================================================

bool EditKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<float>& _keyFrames,
    float _duration,
    float _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        TimelinePopup::DrawValueEditFloat,
        _howEditItem,
        true); // float版はコマンド付きAddNode
}

bool EditKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<OriGine::Vec2f>& _keyFrames,
    float _duration,
    const OriGine::Vec2f& _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        TimelinePopup::DrawValueEditVec2,
        _howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<OriGine::Vec3f>& _keyFrames,
    float _duration,
    const OriGine::Vec3f& _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        TimelinePopup::DrawValueEditVec3,
        _howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<OriGine::Vec4f>& _keyFrames,
    float _duration,
    const OriGine::Vec4f& _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        TimelinePopup::DrawValueEditVec4,
        _howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<OriGine::Quaternion>& _keyFrames,
    float _duration,
    const OriGine::Quaternion& _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        TimelinePopup::DrawValueEditQuaternion,
        _howEditItem,
        false);
}

bool EditColorKeyFrame(
    const std::string& _label,
    OriGine::AnimationCurve<OriGine::Vec4f>& _keyFrames,
    float _duration,
    const OriGine::Vec4f& _defaultValue,
    std::function<void(int)> _howEditItem) {
    return EditColorKeyFrameImpl(
        _label,
        _keyFrames,
        _duration,
        _defaultValue,
        _howEditItem);
}

} // namespace ImGui

#endif // _DEBUG
