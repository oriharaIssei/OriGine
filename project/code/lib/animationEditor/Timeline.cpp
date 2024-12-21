#include "Timeline.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGui {
bool TimeLineButtons(
    const std::string& _label,
    std::vector<float*> _nodeTimes,
    float& _duration,
    std::function<void(float newNodeTime)> _updateOnNodeDragged,
    std::function<void()> _sliderPopupUpdate,
    std::function<void(int nodeIndex)> _nodePopupUpdate,
    std::function<void(float newDuration)> _updateOnDurationChanged) {
    // グループを開始
    ImGui::BeginGroup();

    // 左にラベルを表示
    ImGui::Text("%s", _label.c_str());
    ImGui::SameLine();

    // タイムラインの開始位置を記録
    float timelineStartX = ImGui::GetCursorPosX();

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImDrawList* draw_list   = GetWindowDrawList();

    float durationInputWidth = 32.0f; // 必要に応じて調整

    const ImGuiID sliderId   = GetID(_label.c_str());                // スライダーの ID
    const float sliderWidth  = CalcItemWidth() - durationInputWidth; // スライダーの幅 (duration の入力フィールド分を除く)
    const float sliderHeight = 20.0f;                                // スライダーの高さ

    // スライダー背景の領域を保持するための変数
    ImRect sliderRect = {0.0f, 0.0f, 0.0f, 0.0f};

    ///=================================================================================================
    /// Draw Slider
    ///=================================================================================================
    auto DrawSlider = [&]() {
        // 背景矩形を描画
        const ImVec2 cursorPos = GetCursorScreenPos();
        const ImVec2 sliderSize(sliderWidth, sliderHeight);

        ImVec2 rectEnd(cursorPos.x + sliderSize.x, cursorPos.y + sliderSize.y);
        const ImU32 sliderBgColor =
            IM_COL32(100, 100, 100, 255); // スライダー背景色
        draw_list->AddRectFilled(cursorPos, rectEnd, sliderBgColor, style.FrameRounding);

        sliderRect = ImRect(cursorPos, rectEnd);

        // スライダーの入力領域を登録
        ItemSize(sliderSize, GetStyle().FramePadding.y);
        return 0;
    }();

    if (!ItemAdd(sliderRect, sliderId)) {
        return false;
    }

    // ImGuiのStateStorageを取得
    ImGuiStorage* storage = ImGui::GetStateStorage();

    // ドラッグ中の情報を保持する変数を取得（各タイムラインごとに管理）
    ImGuiID draggedIndexId = sliderId + ImGui::GetID("draggedIndex");
    ImGuiID draggedValueId = sliderId + ImGui::GetID("draggedValue");
    ImGuiID popUpIndexId   = sliderId + ImGui::GetID("popUpIndex");

    int draggedIndex   = storage->GetInt(draggedIndexId, -1);     // ドラッグ中のボタンのインデックス
    float draggedValue = storage->GetFloat(draggedValueId, 0.0f); // ドラッグ中の値
    int popUpIndex     = storage->GetInt(popUpIndexId, -1);       // ポップアップ表示中のボタンのインデックス

    const float buttonSize = 10.0f; // ボタンの幅（ピクセル）

    // 各ボタンを描画
    for (int i = 0; i < _nodeTimes.size(); ++i) {
        float t       = (*_nodeTimes[i]) / (_duration);     // 正規化された位置
        float buttonX = sliderRect.Min.x + t * sliderWidth; // ボタンの位置
        ImVec2 buttonPos(buttonX - buttonSize * 0.5f, sliderRect.Min.y);
        ImVec2 buttonEnd(buttonPos.x + buttonSize, buttonPos.y + sliderHeight);

        // ボタンの位置を取得
        ImRect buttonRect(buttonPos, buttonEnd);
        bool isHovered = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        // ドラッグ処理
        if (isHovered) {
            if (IsMouseClicked(0) && draggedIndex == -1) {
                draggedIndex = i; // ドラッグを開始
                draggedValue = *_nodeTimes[i];
                SetActiveID(sliderId, GetCurrentWindow()); // ボタンをアクティブに設定
                FocusWindow(GetCurrentWindow());
            } else if (IsMouseClicked(1)) {
                popUpIndex = i;
                SetActiveID(sliderId, GetCurrentWindow());
                FocusWindow(GetCurrentWindow());
                OpenPopup(std::string(_label + "node" + std::to_string(i)).c_str());
            }
        }

        bool isActive = (draggedIndex == i);
        if (isActive && IsMouseDragging(0)) {
            // マウス位置から新しい位置を計算
            float newT     = (GetMousePos().x - sliderRect.Min.x) / sliderWidth;
            newT           = ImClamp(newT, 0.0f, 1.0f); // スライダー範囲内にクランプ
            *_nodeTimes[i] = newT * _duration;          // 時間を更新
            if (_updateOnNodeDragged) {
                _updateOnNodeDragged(*_nodeTimes[i]);
            }
        }

        if (isActive && IsMouseReleased(0)) {
            draggedIndex = -1; // ドラッグを終了
        }

        // ボタンを描画
        PushID(i);
        draw_list->AddRectFilled(
            buttonRect.Min,
            buttonRect.Max,
            IM_COL32(200, 200, 200, 255),
            style.FrameRounding);

        // アイテムオーバーラップを許可（ウィンドウ外部との干渉防止）
        SetItemAllowOverlap();
        PopID();
    }

    ///=================================================================================================
    /// PopUpUpdate
    ///=================================================================================================
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
        bool isSliderHovered = IsMouseHoveringRect(sliderRect.Min, sliderRect.Max);
        if (_sliderPopupUpdate) {
            if (isSliderHovered && IsMouseClicked(1)) {
                OpenPopup((_label + "slider").c_str());
            }
            if (BeginPopup((_label + "slider").c_str())) {
                _sliderPopupUpdate();
                EndPopup();
            }
        }
    }

    // ドラッグ状態を保存
    storage->SetInt(draggedIndexId, draggedIndex);
    storage->SetInt(popUpIndexId, popUpIndex);
    storage->SetFloat(draggedValueId, draggedValue);

    // duration を右側に表示
    ImGui::SameLine();

    ///=================================================================================================
    /// Input Duration
    ///=================================================================================================
    auto InputDuration = [&]() {
        // 右端の位置を計算
        float rightEdge = ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x;
        // カーソル位置を右端へ移動
        ImGui::SetCursorPosX(rightEdge - durationInputWidth);

        // duration の入力フィールドを表示
        ImGui::SetNextItemWidth(durationInputWidth);
        if (ImGui::InputFloat("##Duration", &_duration, 0.0f, 0.0f, "%.3f")) {
            if (_updateOnDurationChanged) {
                _updateOnDurationChanged(_duration);
            }
        }
        return 0;
    }();
    ImGui::EndGroup();

    return true;
}
} // namespace ImGui
