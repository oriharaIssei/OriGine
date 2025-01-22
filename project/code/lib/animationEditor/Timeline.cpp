#include "Timeline.h"

///stl
//assert
#include <assert.h>
//algorithm
#include <algorithm>

///lib
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGui {
bool TimeLineButtons(
    const std::string& _label,
    std::vector<float>& _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void(float _currentTime)> _sliderPopupUpdate,
    std::function<bool(int nodeIndex)> _nodePopupUpdate) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + width, window->DC.CursorPos[Y] + 20.0f));
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
    const float sliderWidth = frame_bb.Max[X] - frame_bb.Min[X];
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
            _updateOnNodeDragged(_nodeTimes[draggedIndex]);
            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _nodeTimes.size(); ++i) {
        float t       = (_nodeTimes[i]) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _nodeTimes[i];
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
                float newT    = (GetMousePos()[X] - frame_bb.Min[X]) / sliderWidth;
                newT          = ImClamp(newT, 0.0f, 1.0f);
                _nodeTimes[i] = newT * _duration;
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
                if (!_nodePopupUpdate(popUpIndex)) {
                    popUpIndex = -1;
                }
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
                float currentTime = ((GetMousePos()[X] - frame_bb.Min[X]) / (frame_bb.Max[X] - frame_bb.Min[X])) * _duration;
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

bool TimeLineButtons(
    const std::string& _label,
    std::vector<KeyFrame<int>>& _nodeTimes,
    float _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void(float _currentTime)> _sliderPopupUpdate,
    std::function<bool(int nodeIndex)> _nodePopupUpdate) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + width, window->DC.CursorPos[Y] + 20.0f));
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
    const float sliderWidth = frame_bb.Max[X] - frame_bb.Min[X];
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
            _updateOnNodeDragged(_nodeTimes[draggedIndex].time);
            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _nodeTimes.size(); ++i) {
        float t       = (_nodeTimes[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _nodeTimes[i].time;
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
                float newT         = (GetMousePos()[X] - frame_bb.Min[X]) / sliderWidth;
                newT               = ImClamp(newT, 0.0f, 1.0f);
                _nodeTimes[i].time = newT * _duration;
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
                if (!_nodePopupUpdate(popUpIndex)) {
                    // 螟ｱ謨励＠縺溘ｉ繝昴ャ繝励い繝・・繧帝哩縺倥ｋ
                    popUpIndex = -1;
                }
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
                float currentTime = ((GetMousePos()[X] - frame_bb.Min[X]) / (frame_bb.Max[X] - frame_bb.Min[X])) * _duration;
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

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<float>& _keyFrames,
    float _duration,
    std::function<void(int)> _howEditItem) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos[X] + width, window->DC.CursorPos.y + 20.0f));
    const ImRect total_bb(frame_bb.Min, ImVec2((label_size[X] > 0.0f ? style.ItemInnerSpacing[X] + label_size[X] : 0.0f) + frame_bb.Max[X], frame_bb.Max.y));

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
    const float sliderWidth = frame_bb.Max[X] - frame_bb.Min[X];
    const float buttonSize  = 10.0f;

    ImGuiStorage* storage  = ImGui::GetStateStorage();
    ImGuiID draggedIndexId = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId = id + ImGui::GetID("draggedValue");
    ImGuiID popUpIndexId   = id + ImGui::GetID("popUpIndex");

    int draggedIndex   = storage->GetInt(draggedIndexId, -1);
    float draggedValue = storage->GetFloat(draggedValueId, 0.0f);
    int popUpIndex     = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            //繧ｽ繝ｼ繝医☆繧・
            if (_keyFrames.size() > 1) {
                // 繧ｭ繝ｼ繝輔Ξ繝ｼ繝縺ｫ繧医ｋ 繝弱・繝峨・鬆・分繧貞､画峩
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _keyFrames[i].time;
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
                float newT         = (GetMousePos()[X] - frame_bb.Min[X]) / sliderWidth;
                newT               = ImClamp(newT, 0.0f, 1.0f);
                _keyFrames[i].time = newT * _duration;
            }
        }

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, IM_COL32(200, 200, 200, 255), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        std::string popupId = _label + "node" + std::to_string(popUpIndex);
        if (BeginPopup(popupId.c_str())) {

            auto popupUpdadte = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        _keyFrames[0].time  = 0.0f;
                        _keyFrames[0].value = 0.0f;
                        return 0;
                    }
                    _keyFrames.erase(_keyFrames.begin() + popUpIndex);
                    popUpIndex = 0;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    _keyFrames.push_back(_keyFrames[popUpIndex]);

                    _keyFrames.back().time += 0.01f;
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    popUpIndex += 1;
                    return 0;
                }
                ImGui::Text("Time");
                ImGui::DragFloat(
                    std::string("##Time" + _label + std::to_string(popUpIndex)).c_str(),
                    &_keyFrames[popUpIndex].time,
                    0.1f);

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("Value");
                    ImGui::DragFloat(
                        std::string("##Value" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value,
                        0.1f);
                }

                return 1;
            }();

            EndPopup();
        } else {
            popUpIndex = -1;
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((_label + "slider").c_str());
        }
        if (BeginPopup((_label + "slider").c_str())) {
            float currentTime = ((GetMousePos()[X] - frame_bb.Min[X]) / (frame_bb.Max[X] - frame_bb.Min[X])) * _duration;

            auto sliderPopupUpdate = [&]() {
                // SliderPopup
                if (ImGui::Button("Add Node")) {
                    _keyFrames.push_back(
                        {currentTime, CalculateValue(_keyFrames, currentTime)});
                    ImGui::CloseCurrentPopup();
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    return 0;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return 0;
                }

                return 0;
            }();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec3f>& _keyFrames,
    float _duration,
    std::function<void(int)> _howEditItem) {
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
        if (draggedIndex != -1) {
            //繧ｽ繝ｼ繝医☆繧・
            if (_keyFrames.size() > 1) {
                // 繧ｭ繝ｼ繝輔Ξ繝ｼ繝縺ｫ繧医ｋ 繝弱・繝峨・鬆・分繧貞､画峩
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _keyFrames[i].time;
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
                float newT         = (GetMousePos().x - frame_bb.Min.x) / sliderWidth;
                newT               = ImClamp(newT, 0.0f, 1.0f);
                _keyFrames[i].time = newT * _duration;
            }
        }

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, IM_COL32(200, 200, 200, 255), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        std::string popupId = _label + "node" + std::to_string(popUpIndex);
        if (BeginPopup(popupId.c_str())) {

            auto popupUpdadte = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        _keyFrames[0].time  = 0.0f;
                        _keyFrames[0].value = {0.0f, 0.0f, 0.0f};
                        return 0;
                    }
                    _keyFrames.erase(_keyFrames.begin() + popUpIndex);
                    popUpIndex = 0;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    _keyFrames.push_back(_keyFrames[popUpIndex]);

                    _keyFrames.back().time += 0.01f;
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    popUpIndex += 1;
                    return 0;
                }
                ImGui::Text("Time");
                ImGui::DragFloat(
                    std::string("##Time" + _label + std::to_string(popUpIndex)).c_str(),
                    &_keyFrames[popUpIndex].time,
                    0.1f);

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    ImGui::DragFloat(
                        std::string("##X" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[X],
                        0.1f);
                    ImGui::Text("Y:");

                    ImGui::DragFloat(
                        std::string("##Y" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[Y],
                        0.1f);
                    ImGui::Text("Z:");
                    ImGui::DragFloat(
                        std::string("##Z" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[Z],
                        0.1f);
                }

                return 1;
            }();

            EndPopup();
        } else {
            popUpIndex = -1;
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((_label + "slider").c_str());
        }
        if (BeginPopup((_label + "slider").c_str())) {
            float currentTime = ((GetMousePos().x - frame_bb.Min.x) / (frame_bb.Max.x - frame_bb.Min.x)) * _duration;

            auto sliderPopupUpdate = [&]() {
                // SliderPopup
                if (ImGui::Button("Add Node")) {
                    _keyFrames.push_back(
                        {currentTime, CalculateValue(_keyFrames, currentTime)});
                    ImGui::CloseCurrentPopup();
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    return 0;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return 0;
                }

                return 0;
            }();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration,
    std::function<void(int)> _howEditItem) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + width, window->DC.CursorPos[Y] + 20.0f));
    const ImRect total_bb(frame_bb.Min, ImVec2((label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + frame_bb.Max.x, frame_bb.Max[Y]));

    ItemSize(total_bb, style.FramePadding[Y]);
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
        if (draggedIndex != -1) {
            //ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min[Y]);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos[Y] + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _keyFrames[i].time;
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
                float newT         = (GetMousePos().x - frame_bb.Min.x) / sliderWidth;
                newT               = ImClamp(newT, 0.0f, 1.0f);
                _keyFrames[i].time = newT * _duration;
            }
        }

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, IM_COL32(200, 200, 200, 255), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        std::string popupId = _label + "node" + std::to_string(popUpIndex);
        if (BeginPopup(popupId.c_str())) {

            auto popupUpdadte = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        _keyFrames[0].time  = 0.0f;
                        _keyFrames[0].value = {0.0f, 0.0f, 0.0f, 0.0f};
                        return 0;
                    }
                    _keyFrames.erase(_keyFrames.begin() + popUpIndex);
                    popUpIndex = 0;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    _keyFrames.push_back(_keyFrames[popUpIndex]);

                    _keyFrames.back().time += 0.01f;
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    popUpIndex += 1;
                    return 0;
                }
                ImGui::Text("Time");
                ImGui::DragFloat(
                    std::string("##Time" + _label + std::to_string(popUpIndex)).c_str(),
                    &_keyFrames[popUpIndex].time,
                    0.1f);

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    ImGui::DragFloat(
                        std::string("##X" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[X],
                        0.1f);
                    ImGui::Text("Y:");

                    ImGui::DragFloat(
                        std::string("##Y" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[Y],
                        0.1f);
                    ImGui::Text("Z:");
                    ImGui::DragFloat(
                        std::string("##Z" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[Z],
                        0.1f);
                    ImGui::Text("W:");
                    ImGui::DragFloat(
                        std::string("##W" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value[W],
                        0.1f);
                }

                return 1;
            }();

            EndPopup();
        } else {
            popUpIndex = -1;
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((_label + "slider").c_str());
        }
        if (BeginPopup((_label + "slider").c_str())) {
            float currentTime = ((GetMousePos()[X] - frame_bb.Min[X]) / (frame_bb.Max[X] - frame_bb.Min[X])) * _duration;

            auto sliderPopupUpdate = [&]() {
                // SliderPopup
                if (ImGui::Button("Add Node")) {
                    _keyFrames.push_back(
                        {currentTime, CalculateValue(_keyFrames, currentTime)});
                    ImGui::CloseCurrentPopup();
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    return 0;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return 0;
                }

                return 0;
            }();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Quaternion>& _keyFrames,
    float _duration,
    std::function<void(int)> _howEditItem) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(_label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(_label.c_str(), NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos[X] + width, window->DC.CursorPos[Y] + 20.0f));
    const ImRect total_bb(frame_bb.Min, ImVec2((label_size[X] > 0.0f ? style.ItemInnerSpacing[X] + label_size[X] : 0.0f) + frame_bb.Max[X], frame_bb.Max[Y]));

    ItemSize(total_bb, style.FramePadding[Y]);
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
    const float sliderWidth = frame_bb.Max[X] - frame_bb.Min[X];
    const float buttonSize  = 10.0f;

    ImGuiStorage* storage  = ImGui::GetStateStorage();
    ImGuiID draggedIndexId = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId = id + ImGui::GetID("draggedValue");
    ImGuiID popUpIndexId   = id + ImGui::GetID("popUpIndex");

    int draggedIndex   = storage->GetInt(draggedIndexId, -1);
    float draggedValue = storage->GetFloat(draggedValueId, 0.0f);
    int popUpIndex     = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            //ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < _keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min[Y]);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos[Y] + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i;
                draggedValue = _keyFrames[i].time;
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
                float newT         = (GetMousePos()[X] - frame_bb.Min[X]) / sliderWidth;
                newT               = ImClamp(newT, 0.0f, 1.0f);
                _keyFrames[i].time = newT * _duration;
            }
        }

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, IM_COL32(200, 200, 200, 255), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        std::string popupId = _label + "node" + std::to_string(popUpIndex);
        if (BeginPopup(popupId.c_str())) {

            auto popupUpdadte = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        _keyFrames[0].time  = 0.0f;
                        _keyFrames[0].value = {0.0f, 0.0f, 0.0f, 1.0f};
                        return 0;
                    }
                    _keyFrames.erase(_keyFrames.begin() + popUpIndex);
                    popUpIndex = 0;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    _keyFrames.push_back(_keyFrames[popUpIndex]);

                    _keyFrames.back().time += 0.01f;
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    popUpIndex += 1;
                    return 0;
                }
                ImGui::Text("Time");
                ImGui::DragFloat(
                    std::string("##Time" + _label + std::to_string(popUpIndex)).c_str(),
                    &_keyFrames[popUpIndex].time,
                    0.1f);

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    ImGui::DragFloat(
                        std::string("##X" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value.x,
                        0.1f);
                    ImGui::Text("Y:");

                    ImGui::DragFloat(
                        std::string("##Y" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value.y,
                        0.1f);
                    ImGui::Text("Z:");
                    ImGui::DragFloat(
                        std::string("##Z" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value.z,
                        0.1f);
                    ImGui::Text("W:");
                    ImGui::DragFloat(
                        std::string("##W" + _label + std::to_string(popUpIndex)).c_str(),
                        &_keyFrames[popUpIndex].value.w,
                        0.1f);
                    _keyFrames[popUpIndex].value = _keyFrames[popUpIndex].value.normalize();
                }
                return 1;
            }();

            EndPopup();
        } else {
            popUpIndex = -1;
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((_label + "slider").c_str());
        }
        if (BeginPopup((_label + "slider").c_str())) {
            float currentTime = ((GetMousePos()[X] - frame_bb.Min[X]) / (frame_bb.Max[X] - frame_bb.Min[X])) * _duration;

            auto sliderPopupUpdate = [&]() {
                // SliderPopup
                if (ImGui::Button("Add Node")) {
                    _keyFrames.push_back(
                        {currentTime, CalculateValue(_keyFrames, currentTime)});
                    ImGui::CloseCurrentPopup();
                    std::sort(
                        _keyFrames.begin(),
                        _keyFrames.end(),
                        [](const auto& a, const auto& b) {
                            return a.time < b.time;
                        });
                    return 0;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return 0;
                }

                return 0;
            }();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

} // namespace ImGui
