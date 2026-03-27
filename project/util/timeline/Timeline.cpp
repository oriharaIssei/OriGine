#include "Timeline.h"


#ifdef _DEBUG

/// stl
#include <algorithm>

/// timeline modules
#include "TimelineButtons.h"
#include "TimelineConfig.h"
#include "TimelineCore.h"
#include "TimelineEditor.h"
#include "TimelinePopup.h"

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
    const std::string& label,
    std::vector<float>& nodeTimes,
    float duration,
    std::function<void(float newNodeTime)> updateOnNodeDragged,
    std::function<void(float currentTime)> sliderPopupUpdate,
    std::function<bool(int nodeIndex)> nodePopupUpdate) {
    return TimeLineButtonsImpl(
        label,
        nodeTimes,
        duration,
        [](const float& time) { return time; },
        [](float& time, float newTime) { time = newTime; },
        updateOnNodeDragged,
        sliderPopupUpdate,
        nodePopupUpdate);
}

bool TimeLineButtons(
    const std::string& label,
    OriGine::AnimationCurve<int>& keyFrames,
    float duration,
    std::function<void(float newNodeTime)> updateOnNodeDragged,
    std::function<void(float currentTime)> sliderPopupUpdate,
    std::function<bool(int nodeIndex)> nodePopupUpdate) {
    return TimeLineButtonsImpl(
        label,
        keyFrames,
        duration,
        [](const OriGine::KeyFrame<int>& kf) { return kf.time; },
        [](OriGine::KeyFrame<int>& kf, float newTime) { kf.time = newTime; },
        updateOnNodeDragged,
        sliderPopupUpdate,
        nodePopupUpdate);
}

//==============================================================================
// EditKeyFrame 実装
//==============================================================================

bool EditKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<float>& keyFrames,
    float duration,
    float defaultValue,
    std::function<void(int)> howEditItem) {
    return EditKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        TimelinePopup::DrawValueEditFloat,
        howEditItem,
        true); // float版はコマンド付きAddNode
}

bool EditKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec2f>& keyFrames,
    float duration,
    const OriGine::Vec2f& defaultValue,
    std::function<void(int)> howEditItem) {
    return EditKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        TimelinePopup::DrawValueEditVec2,
        howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec3f>& keyFrames,
    float duration,
    const OriGine::Vec3f& defaultValue,
    std::function<void(int)> howEditItem) {
    return EditKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        TimelinePopup::DrawValueEditVec3,
        howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec4f>& keyFrames,
    float duration,
    const OriGine::Vec4f& defaultValue,
    std::function<void(int)> howEditItem) {
    return EditKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        TimelinePopup::DrawValueEditVec4,
        howEditItem,
        false);
}

bool EditKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Quaternion>& keyFrames,
    float duration,
    const OriGine::Quaternion& defaultValue,
    std::function<void(int)> howEditItem) {
    return EditKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        TimelinePopup::DrawValueEditQuaternion,
        howEditItem,
        false);
}

bool EditColorKeyFrame(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec4f>& keyFrames,
    float duration,
    const OriGine::Vec4f& defaultValue,
    std::function<void(int)> howEditItem) {
    return EditColorKeyFrameImpl(
        label,
        keyFrames,
        duration,
        defaultValue,
        howEditItem);
}

} // namespace ImGui

#endif // _DEBUG
