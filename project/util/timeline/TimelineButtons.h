#pragma once

/// @file TimelineButtons.h
/// @brief タイムラインボタン（シンプルなノード表示）のテンプレート実装

#ifdef _DEBUG

#include "TimelineConfig.h"
#include "TimelineCore.h"

#include <functional>
#include <string>
#include <vector>

namespace ImGui {

/// @brief TimeLineButtonsのテンプレート実装
/// @tparam T コンテナの要素型（float または KeyFrame<int>）
/// @tparam GetTime 要素から時間を取得する関数
/// @param label ラベル
/// @param nodes ノードコンテナ
/// @param duration アニメーション全体時間
/// @param getTime 要素から時間を取得する関数
/// @param setTime 要素に時間を設定する関数
/// @param updateOnNodeDragged ノードドラッグ時のコールバック
/// @param sliderPopupUpdate スライダーポップアップ更新関数
/// @param nodePopupUpdate ノードポップアップ更新関数
/// @return 更新されたか
template <typename Container, typename GetTimeFunc, typename SetTimeFunc>
bool TimeLineButtonsImpl(
    const std::string& label,
    Container& nodes,
    float duration,
    GetTimeFunc getTime,
    SetTimeFunc setTime,
    std::function<void(float newNodeTime)> updateOnNodeDragged = nullptr,
    std::function<void(float currentTime)> sliderPopupUpdate   = nullptr,
    std::function<bool(int nodeIndex)> nodePopupUpdate         = nullptr) {
    using namespace TimelineCore;
    using namespace ::TimelineConfig;

    // ウィジェット初期化
    WidgetInitData widgetData = InitializeWidget(label);
    if (!widgetData.isValid) {
        return false;
    }

    ImDrawList* drawList = GetWindowDrawList();

    // ストレージキー生成と状態読み込み
    StorageKeys keys    = CreateStorageKeys(widgetData.id);
    TimelineState state = LoadState(keys);

    // マウスリリース時のドラッグ終了処理
    if (IsMouseReleased(0)) {
        if (state.draggedIndex != -1 && updateOnNodeDragged) {
            updateOnNodeDragged(getTime(nodes[state.draggedIndex]));
            state.draggedIndex = -1;
        }
    }

    // ノードの描画とインタラクション
    for (int i = 0; i < (int)nodes.size(); ++i) {
        float nodeTime    = getTime(nodes[i]);
        ImRect buttonRect = CalculateNodeRect(widgetData.frameBB, widgetData.sliderWidth, nodeTime, duration);
        bool isHovered    = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && state.draggedIndex == -1) {
                state.draggedIndex = i;
                state.draggedValue = nodeTime;
                SetActiveID(widgetData.id, widgetData.window);
                FocusWindow(widgetData.window);
            } else if (IsMouseClicked(1)) {
                state.popUpIndex = i;
                SetActiveID(widgetData.id, widgetData.window);
                FocusWindow(widgetData.window);
                OpenPopup(std::string(label + "node" + std::to_string(i)).c_str());
            }
        }

        // ドラッグ処理
        if (state.draggedIndex == i && IsMouseDragging(0)) {
            float newT = CalculateNormalizedTimeFromMouse(widgetData.frameBB);
            setTime(nodes[i], newT * duration);
        }

        PushID(i);
        DrawNode(drawList, buttonRect);
        SetItemAllowOverlap();
        PopID();
    }

    // ノードポップアップ処理
    if (state.popUpIndex != -1) {
        if (nodePopupUpdate) {
            std::string popupId = label + "node" + std::to_string(state.popUpIndex);
            if (BeginPopup(popupId.c_str())) {
                if (!nodePopupUpdate(state.popUpIndex)) {
                    state.popUpIndex = -1;
                }
                EndPopup();
            } else {
                state.popUpIndex = -1;
            }
        }
    } else {
        // スライダーポップアップ
        bool isSliderHovered = IsMouseHoveringRect(widgetData.frameBB.Min, widgetData.frameBB.Max);
        if (sliderPopupUpdate) {
            if (isSliderHovered && IsMouseClicked(1)) {
                OpenPopup((label + "slider").c_str());
            }
            if (BeginPopup((label + "slider").c_str())) {
                float currentTime = CalculateTimeFromMouse(widgetData.frameBB, duration);
                sliderPopupUpdate(currentTime);
                EndPopup();
            }
        }
    }

    // 状態保存
    SaveState(keys, state);

    return true;
}

} // namespace ImGui

#endif // _DEBUG
