#pragma once

/// @file TimelineEditor.h
/// @brief タイムラインエディタのテンプレート実装

#ifdef _DEBUG

#include "component/animation/AnimationData.h"
#include "TimelineConfig.h"
#include "TimelineCore.h"
#include "TimelinePopup.h"

#include <algorithm>
#include <functional>

namespace ImGui {

/// @brief EditKeyFrameのテンプレート実装
/// @tparam T 値の型
/// @tparam DrawValueEdit 値編集UIを描画する関数型
/// @param label ラベル
/// @param keyFrames キーフレーム配列
/// @param duration アニメーション全体時間
/// @param defaultValue デフォルト値
/// @param drawValueEdit 値編集UIを描画する関数
/// @param howEditItem カスタム編集関数（nullptrの場合はデフォルト使用）
/// @param useCommandForAddNode ノード追加時にコマンドを使用するか
/// @return 更新されたか
template <typename T, typename DrawValueEditFunc>
bool EditKeyFrameImpl(
    const std::string& label,
    OriGine::AnimationCurve<T>& keyFrames,
    float duration,
    const T& defaultValue,
    DrawValueEditFunc drawValueEdit,
    std::function<void(int)> howEditItem = nullptr,
    bool useCommandForAddNode            = false) {
    using namespace TimelineCore;
    using namespace ::TimelineConfig;
    using namespace ::TimelinePopup;

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
        if (state.draggedIndex != -1) {
            // ソート
            if (keyFrames.size() > 1) {
                SortKeyFrames(keyFrames);
            }

            // Undo/Redoコマンド発行
            if (state.draggedValue != state.startDragValue) {
                keyFrames[state.draggedIndex].time = state.startDragValue;
                auto command                       = std::make_unique<SetterCommand<float>>(
                    &keyFrames[state.draggedIndex].time,
                    state.draggedValue);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }

            state.draggedIndex = -1;
        }
    }

    // ノードの描画とインタラクション
    for (int i = 0; i < (int)keyFrames.size(); ++i) {
        ImRect buttonRect = CalculateNodeRect(widgetData.frameBB, widgetData.sliderWidth, keyFrames[i].time, duration);
        bool isHovered    = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && state.draggedIndex == -1) {
                state.draggedIndex   = i;
                state.startDragValue = keyFrames[i].time;
                state.draggedValue   = state.startDragValue;
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
            float newT         = CalculateNormalizedTimeFromMouse(widgetData.frameBB);
            keyFrames[i].time  = newT * duration;
            state.draggedValue = keyFrames[i].time;
        }

        PushID(i);
        DrawNode(drawList, buttonRect);
        SetItemAllowOverlap();
        PopID();
    }

    // ノードポップアップ処理
    if (state.popUpIndex != -1) {
        std::string popupId = label + "node" + std::to_string(state.popUpIndex);
        if (BeginPopup(popupId.c_str())) {
            ImGui::Text("NodeNumber : %d", state.popUpIndex);

            if (ImGui::Button("Delete")) {
                HandleDeleteNode(keyFrames, state.popUpIndex, defaultValue);
            }
            if (ImGui::Button("Copy")) {
                HandleCopyNode(keyFrames, state.popUpIndex, keys.popUpIndexId);
            }

            DrawTimeEdit(label, keyFrames, state.popUpIndex);
            ImGui::Spacing();

            if (howEditItem) {
                howEditItem(state.popUpIndex);
            } else {
                drawValueEdit(label, keyFrames, state.popUpIndex);
            }

            EndPopup();
        } else {
            state.popUpIndex = -1;
        }
    } else {
        // スライダーポップアップ（ノード追加）
        bool isSliderHovered = IsMouseHoveringRect(widgetData.frameBB.Min, widgetData.frameBB.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((label + "slider").c_str());
        }
        if (BeginPopup((label + "slider").c_str())) {
            float currentTime = CalculateTimeFromMouse(widgetData.frameBB, duration);

            if (ImGui::Button("Add Node")) {
                HandleAddNode(keyFrames, currentTime, keys.popUpIndexId, useCommandForAddNode);
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            EndPopup();
        }
    }

    // 状態保存
    SaveState(keys, state);

    return true;
}

/// @brief Color用EditKeyFrameの実装（ノード描画がカスタム）
/// @param label ラベル
/// @param keyFrames キーフレーム配列
/// @param duration アニメーション全体時間
/// @param defaultValue デフォルト値
/// @param howEditItem カスタム編集関数
/// @return 更新されたか
inline bool EditColorKeyFrameImpl(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec4f>& keyFrames,
    float duration,
    const OriGine::Vec4f& defaultValue,
    std::function<void(int)> howEditItem = nullptr) {
    using namespace TimelineCore;
    using namespace ::TimelineConfig;
    using namespace ::TimelinePopup;

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
        if (state.draggedIndex != -1) {
            if (keyFrames.size() > 1) {
                SortKeyFrames(keyFrames);
            }

            if (state.draggedValue != state.startDragValue) {
                keyFrames[state.draggedIndex].time = state.startDragValue;
                auto command                       = std::make_unique<SetterCommand<float>>(
                    &keyFrames[state.draggedIndex].time,
                    state.draggedValue);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }

            state.draggedIndex = -1;
        }
    }

    // ノードの描画とインタラクション
    for (int i = 0; i < (int)keyFrames.size(); ++i) {
        ImRect buttonRect = CalculateNodeRect(widgetData.frameBB, widgetData.sliderWidth, keyFrames[i].time, duration);
        bool isHovered    = IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

        if (isHovered) {
            if (IsMouseClicked(0) && state.draggedIndex == -1) {
                state.draggedIndex   = i;
                state.draggedValue   = keyFrames[i].time;
                state.startDragValue = state.draggedValue;
                SetActiveID(widgetData.id, widgetData.window);
                FocusWindow(widgetData.window);
            } else if (IsMouseClicked(1)) {
                state.popUpIndex = i;
                SetActiveID(widgetData.id, widgetData.window);
                FocusWindow(widgetData.window);
                OpenPopup(std::string(label + "node" + std::to_string(i)).c_str());
            }
        }

        if (state.draggedIndex == i && IsMouseDragging(0)) {
            float newT         = CalculateNormalizedTimeFromMouse(widgetData.frameBB);
            keyFrames[i].time  = newT * duration;
            state.draggedValue = keyFrames[i].time;
        }

        // カラーノードとして描画
        ImVec4 nodeColor = ImVec4(
            keyFrames[i].value[OriGine::X],
            keyFrames[i].value[OriGine::Y],
            keyFrames[i].value[OriGine::Z],
            (std::max)(keyFrames[i].value[OriGine::W], NODE_MIN_ALPHA));

        PushID(i);
        DrawColorNode(drawList, buttonRect, nodeColor);
        SetItemAllowOverlap();
        PopID();
    }

    // ノードポップアップ処理
    if (state.popUpIndex != -1) {
        std::string popupId = label + "node" + std::to_string(state.popUpIndex);
        if (BeginPopup(popupId.c_str())) {
            ImGui::Text("NodeNumber : %d", state.popUpIndex);

            if (ImGui::Button("Delete")) {
                HandleDeleteNode(keyFrames, state.popUpIndex, defaultValue);
            }
            if (ImGui::Button("Copy")) {
                HandleCopyNode(keyFrames, state.popUpIndex, keys.popUpIndexId);
            }

            DrawTimeEdit(label, keyFrames, state.popUpIndex);
            ImGui::Spacing();

            if (howEditItem) {
                howEditItem(state.popUpIndex);
            } else {
                DrawValueEditColor(label, keyFrames, state.popUpIndex);
            }

            EndPopup();
        } else {
            state.popUpIndex = -1;
        }
    } else {
        bool isSliderHovered = IsMouseHoveringRect(widgetData.frameBB.Min, widgetData.frameBB.Max);
        if (isSliderHovered && IsMouseClicked(1)) {
            OpenPopup((label + "slider").c_str());
        }
        if (BeginPopup((label + "slider").c_str())) {
            float currentTime = CalculateTimeFromMouse(widgetData.frameBB, duration);

            if (ImGui::Button("Add Node")) {
                HandleAddNode(keyFrames, currentTime, keys.popUpIndexId, false);
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            EndPopup();
        }
    }

    SaveState(keys, state);
    return true;
}

} // namespace ImGui

#endif // _DEBUG
