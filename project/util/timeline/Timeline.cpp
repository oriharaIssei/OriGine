#include "Timeline.h"

/// stl
// assert
#include <assert.h>
// algorithm
#include <algorithm>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"


#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "myGui/MyGui.h"

#ifdef _DEBUG

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
    AnimationCurve<int>& _keyFrames,
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
            _updateOnNodeDragged(_keyFrames[draggedIndex].time);
            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
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
        if (_nodePopupUpdate) {
            std::string popupId = _label + "node" + std::to_string(popUpIndex);
            if (BeginPopup(popupId.c_str())) {
                if (!_nodePopupUpdate(popUpIndex)) {
                    // 失敗したらポップアップを閉じる
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
    float _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            // ここで undo/redo コマンドを push
            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);

                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                startDragValue = _keyFrames[i].time;
                draggedValue   = startDragValue;
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
                _keyFrames[i].time = float(newT * _duration);
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

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    // キーフレームを削除
                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<float>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<float>(0.0f, _defaultValue)));
                        return 0;
                    }
                    // キーフレームを削除
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<float>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    popUpIndex = -1;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<float>>>(&_keyFrames, _keyFrames[popUpIndex]));
                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));

                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex + 1);

                            std::sort(
                                _keyFrames.begin(),
                                _keyFrames.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        false);
                    commandCombo->SetFuncOnAfterUndoCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex);

                            std::sort(
                                _keyFrames.begin(),
                                _keyFrames.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        });

                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    return 0;
                }
                ImGui::Text("Time");
                DragGuiCommand<float>("##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("Value");
                    DragGuiCommand("##Value" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value,
                        0.1f);
                }
                return 0;
            };
            popupUpdate();

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
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<float>>>(&_keyFrames, KeyFrame<float>(currentTime, CalculateValue::Linear(_keyFrames, currentTime))));
                    commandCombo->AddCommand(std::make_shared<SortCommand<AnimationCurve<float>>>(&_keyFrames, [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    }));
                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, _keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, (int)_keyFrames.size() - 1);
                        },
                        true);
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    ImGui::CloseCurrentPopup();
                    return 0;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return 0;
                }

                return 0;
            };

            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec2f>& _keyFrames,
    float _duration,
    const Vec2f& _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                startDragValue = _keyFrames[i].time;
                draggedValue   = startDragValue;
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
                _keyFrames[i].time = float(newT * _duration);
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

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<Vec2f>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<Vec2f>(0.0f, _defaultValue)));
                        return 0;
                    }
                    auto commandCombo = std::make_unique<CommandCombo>();
                    // キーフレームを削除
                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<Vec2f>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    popUpIndex = -1;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec2f>>>(&_keyFrames, _keyFrames[popUpIndex]));
                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));

                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex + 1);
                            std::sort(
                                _keyFrames.begin(),
                                _keyFrames.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        false);
                    commandCombo->SetFuncOnAfterUndoCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex);
                            std::sort(
                                _keyFrames.begin(),
                                _keyFrames.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        });
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    return 0;
                }

                ImGui::Text("Time");
                DragGuiCommand<float>(
                    "##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("Value");
                    DragGuiVectorCommand<2, float>(
                        "##Value" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value,
                        0.1f);
                }
                return 0;
            };
            popupUpdate();

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
                        {currentTime, CalculateValue::Linear(_keyFrames, currentTime)});
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
            };

            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec3f>& _keyFrames,
    float _duration,
    const Vec3f& _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min.y);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos.y + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                startDragValue = _keyFrames[i].time;
                draggedValue   = startDragValue;
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
                // キーフレームの時間を変更
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

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    // キーフレームを削除
                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<Vec3f>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<Vec3f>(0.0f, _defaultValue)));
                        return 0;
                    }
                    // キーフレームを削除
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<Vec3f>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));

                    popUpIndex = -1;
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();

                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec3f>>>(&_keyFrames, _keyFrames[popUpIndex]));

                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));
                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, _keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, (int)_keyFrames.size() - 1);
                        },
                        true);
                    return 0;
                }
                ImGui::Text("Time");
                DragGuiCommand<float>(
                    "##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    DragGuiCommand<float>(
                        "##X" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[X],
                        0.1f);
                    ImGui::Text("Y:");
                    DragGuiCommand<float>(
                        "##Y" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[Y],
                        0.1f);
                    ImGui::Text("Z:");
                    DragGuiCommand<float>(
                        "##Z" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[Z],
                        0.1f);
                }

                return 1;
            };
            popupUpdate();
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
                        {currentTime, CalculateValue::Linear(_keyFrames, currentTime)});
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
            };
            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration,
    const Vec4f& _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min[Y]);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos[Y] + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                startDragValue = _keyFrames[i].time;
                draggedValue   = startDragValue;
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

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<Vec4f>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<Vec4f>(0.0f, _defaultValue)));
                        return 0;
                    }
                    auto commandCombo = std::make_unique<CommandCombo>();
                    // キーフレームを削除
                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<Vec4f>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));

                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    popUpIndex = -1;

                    ImGui::CloseCurrentPopup();
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec4f>>>(&_keyFrames, _keyFrames[popUpIndex]));
                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));

                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, (int)_keyFrames.size() - 1);
                        },
                        true);

                    commandCombo->SetFuncOnAfterUndoCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex);
                        });
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    return 0;
                }
                ImGui::Text("Time");
                DragGuiCommand<float>(
                    "##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    DragGuiCommand<float>(
                        "##X" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[X],
                        0.1f);
                    ImGui::Text("Y:");
                    DragGuiCommand<float>(
                        "##Y" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[Y],
                        0.1f);
                    ImGui::Text("Z:");
                    DragGuiCommand<float>(
                        "##Z" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[Z],
                        0.1f);
                    ImGui::Text("W:");
                    DragGuiCommand<float>(
                        "##W" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value[W],
                        0.1f);
                }

                return 1;
            };
            popupUpdate();

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
                        {currentTime, CalculateValue::Linear(_keyFrames, currentTime)});
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
            };
            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditKeyFrame(
    const std::string& _label,
    AnimationCurve<Quaternion>& _keyFrames,
    float _duration,
    const Quaternion& _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min[X] + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min[Y]);
        ImVec2 buttonEnd(buttonPos[X] + buttonSize, buttonPos[Y] + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                startDragValue = _keyFrames[i].time;
                draggedValue   = startDragValue;
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
                float newT = (GetMousePos()[X] - frame_bb.Min[X]) / sliderWidth;
                newT       = ImClamp(newT, 0.0f, 1.0f);

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

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    // キーフレームを削除

                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<Quaternion>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<Quaternion>(0.0f, _defaultValue)));
                        return 0;
                    }
                    auto commandCombo = std::make_unique<CommandCombo>();
                    // キーフレームを削除
                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<Quaternion>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));

                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    popUpIndex = -1;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<Quaternion>>>(&_keyFrames, _keyFrames[popUpIndex]));

                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));

                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, (int)_keyFrames.size() - 1);
                        },
                        true);

                    commandCombo->SetFuncOnAfterUndoCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex);
                        });

                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));
                    return 0;
                }
                ImGui::Text("Time");
                DragGuiCommand<float>(
                    "##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ImGui::Text("X:");
                    DragGuiCommand<float>(
                        "##X" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value.v[X],
                        0.1f);
                    ImGui::Text("Y:");
                    DragGuiCommand<float>(
                        "##Y" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value.v[Y],
                        0.1f);
                    ImGui::Text("Z:");
                    DragGuiCommand<float>(
                        "##Z" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value.v[Z],
                        0.1f);
                    ImGui::Text("W:");
                    DragGuiCommand<float>(
                        "##W" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value.v[W],
                        0.1f);
                    _keyFrames[popUpIndex].value = _keyFrames[popUpIndex].value.normalize();
                }
                // Quaternionは正規化する
                _keyFrames[popUpIndex].value = _keyFrames[popUpIndex].value.normalize();
                return 1;
            };
            popupUpdate();

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
                        {currentTime, CalculateValue::Linear(_keyFrames, currentTime)});
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
            };
            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

bool EditColorKeyFrame(
    const std::string& _label,
    AnimationCurve<Vec4f>& _keyFrames,
    float _duration,
    const Vec4f& _defaultValue,
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

    ImGuiStorage* storage    = ImGui::GetStateStorage();
    ImGuiID draggedIndexId   = id + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId   = id + ImGui::GetID("draggedValue"); // Drag中の値
    ImGuiID startDragValueId = id + ImGui::GetID("startDragValue"); // Drag開始時の値
    ImGuiID popUpIndexId     = id + ImGui::GetID("popUpIndex");

    int draggedIndex     = storage->GetInt(draggedIndexId, -1);
    float draggedValue   = storage->GetFloat(draggedValueId, 0.0f);
    float startDragValue = storage->GetFloat(startDragValueId, 0.f);
    int popUpIndex       = storage->GetInt(popUpIndexId, -1);

    if (IsMouseReleased(0)) {
        if (draggedIndex != -1) {
            // ソートする
            if (_keyFrames.size() > 1) {
                // キーフレームによる ノードの順番を変更
                std::sort(
                    _keyFrames.begin(),
                    _keyFrames.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            }

            if (draggedValue != startDragValue) {
                _keyFrames[draggedIndex].time = startDragValue;
                auto command                  = std::make_unique<SetterCommand<float>>(
                    &_keyFrames[draggedIndex].time,
                    draggedValue);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            draggedIndex = -1;
        }
    }

    for (int i = 0; i < (int)_keyFrames.size(); ++i) {
        float t       = (_keyFrames[i].time) / (_duration);
        float buttonX = frame_bb.Min.x + t * sliderWidth;
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, frame_bb.Min[Y]);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos[Y] + 20.0f);

        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex   = i;
                draggedValue   = _keyFrames[i].time;
                startDragValue = draggedValue;
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

        ImVec4 nodeColor = ImVec4(_keyFrames[i].value[X], _keyFrames[i].value[Y], _keyFrames[i].value[Z], (std::max)(_keyFrames[i].value[W], 0.1f));

        PushID(i);
        draw_list->AddRectFilled(buttonRect.Min, buttonRect.Max, ImColor(nodeColor), style.FrameRounding);
        SetItemAllowOverlap();
        PopID();
    }

    if (popUpIndex != -1) {
        std::string popupId = _label + "node" + std::to_string(popUpIndex);
        if (BeginPopup(popupId.c_str())) {

            auto popupUpdate = [&]() {
                // NodeUpdate
                ImGui::Text("NodeNumber : %d", popUpIndex);

                if (ImGui::Button("Delete")) {
                    // キーフレームを削除

                    if (_keyFrames.size() <= 1) {
                        // 最後のキーフレームを削除する場合は、デフォルト値を設定 (最低でも1つ以上要素を確保する)
                        EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<KeyFrame<Vec4f>>>(
                            &_keyFrames[popUpIndex],
                            KeyFrame<Vec4f>(0.0f, _defaultValue)));
                        popUpIndex = -1;
                        return 0;
                    }

                    // キーフレームを削除

                    auto commandCombo = std::make_unique<CommandCombo>();

                    commandCombo->AddCommand(std::make_shared<EraseElementCommand<AnimationCurve<Vec4f>>>(&_keyFrames, _keyFrames.begin() + popUpIndex));
                    
                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    popUpIndex = -1;
                    return 0;
                }
                if (ImGui::Button("Copy")) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->AddCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec4f>>>(&_keyFrames, _keyFrames[popUpIndex]));
                    commandCombo->AddCommand(std::make_shared<SetterCommand<float>>(&_keyFrames.back().time, _keyFrames.back().time + 0.01f));
                    commandCombo->SetFuncOnAfterCommand(
                        [popUpIndexId, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, (int)_keyFrames.size() - 1);
                        },
                        false);

                    commandCombo->SetFuncOnAfterUndoCommand(
                        [popUpIndexId, popUpIndex, &_keyFrames]() {
                            ImGuiStorage* _storage = ImGui::GetStateStorage();
                            _storage->SetInt(popUpIndexId, popUpIndex);
                        });

                    EditorController::GetInstance()->PushCommand(std::move(commandCombo));

                    return 0;
                }
                ImGui::Text("Time");
                DragGuiCommand<float>(
                    "##Time" + _label + std::to_string(popUpIndex),
                    _keyFrames[popUpIndex].time,
                    0.1f, {}, {}, "%.3f",
                    [&_keyFrames](float* /*val*/) {
                        std::sort(
                            _keyFrames.begin(),
                            _keyFrames.end(),
                            [](const auto& a, const auto& b) {
                                return a.time < b.time;
                            });
                    });

                ImGui::Spacing();

                if (_howEditItem) {
                    _howEditItem(popUpIndex);
                } else {
                    ColorEditGuiCommand<4>(
                        "Color##" + _label + std::to_string(popUpIndex),
                        _keyFrames[popUpIndex].value);
                }

                return 1;
            };
            popupUpdate();

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
                        {currentTime, CalculateValue::Linear(_keyFrames, currentTime)});
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
            };
            sliderPopupUpdate();

            EndPopup();
        }
    }

    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(startDragValueId, startDragValue);
    storage->SetFloat(draggedValueId, draggedValue);

    return true;
}

} // namespace ImGui

#endif // _DEBUG
