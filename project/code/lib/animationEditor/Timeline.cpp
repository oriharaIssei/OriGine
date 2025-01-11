#include "Timeline.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <assert.h>
#include <queue>

namespace ImGui {
bool TimeLineButtons(
    const std::string& _label,
    std::vector<float*> _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void(float _currentTime)> _sliderPopupUpdate,
    std::function<void(int nodeIndex)> _nodePopupUpdate) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + width, window->DC.CursorPos.y + 20.0f));
    const ImRect total_bb(frame_bb.Min, ImVec2((label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + frame_bb.Max.x, frame_bb.Max.y));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb)) {
        return false;
    }

    const bool hovered        = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
    bool temp_input_is_active = TempInputIsActive(id);
    if (!temp_input_is_active) {
        const bool clicked     = hovered && IsMouseClicked(0, id);
        const bool make_active = (clicked || g.NavActivateId == id);
        if (make_active && clicked)
            SetKeyOwner(ImGuiKey_MouseLeft, id);
        if (make_active) {
            SetActiveID(id, window);
            SetFocusID(id, window);
            FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }

    if (temp_input_is_active) {
        return false;
    }

    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered
                                                                                            : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    ImDrawList* draw_list   = GetWindowDrawList();
    const float sliderWidth = frame_bb.Max.x - frame_bb.Min.x;
    const float buttonSize  = 10.0f;

    ImGuiStorage* storage  = ImGui::GetStateStorage();
    ImGuiID draggedIndexId = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId = id + ImGui::GetID("draggedValue");
    ImGuiID popUpIndexId   = id + ImGui::GetID("popUpIndex");

    int draggedIndex   = storage->GetInt(draggedIndexId, -1);
    float draggedValue = storage->GetFloat(draggedValueId, 0.0f);
    int popUpIndex     = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1 && _updateOnNodeDragged) {
            _updateOnNodeDragged(*_nodeTimes[draggedIndex]);
            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _nodeTimes.size(); ++i) {
        float t       = (*_nodeTimes[i]) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = *_nodeTimes[i];
                SetActiveID(id, window);
                FocusWindow(window);
            } else if (IsMouseClicked(1)) {
                popUpIndex = i;
                SetActiveID(id, window);
                FocusWindow(window);
                OpenPopup(std::string(_label + "node" + std::to_string(i)).c_str());
            }
        }

        bool isActive = (draggedIndex == i);
        if (isActive) {
            if (IsMouseDragging(0)) {
                float newT     = (GetMousePos().x - frame_bb.Min.x) / sliderWidth;
                newT           = ImClamp(newT, 0.0f, 1.0f);
                *_nodeTimes[i] = newT * _duration;
            }
        }

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, IM_COL32(200, 200, 200, 255), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        if (_nodePopupUpdate) {
            std::string popupId = _label + "node" + std::to_string(popUpIndex);
            if (BeginPopup(popupId.c_str())) {
                _nodePopupUpdate(popUpIndex);
                EndPopup();
            } else {
                popUpIndex = -1;
            }
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);
        if (_sliderPopupUpdate) {
            if (isSliderHovered && IsMouseClicked(1)) {
                OpenPopup((_label + "slider").c_str());
            }
            if (BeginPopup((_label + "slider").c_str())) {
                float currentTime = ((GetMousePos().x - frame_bb.Min.x) / (frame_bb.Max.x - frame_bb.Min.x)) * _duration;
                _sliderPopupUpdate(currentTime);
                EndPopup();
            }
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

} // namespace ImGui
