#pragma once

/// @file TimelineCore.h
/// @brief タイムラインエディタの共通処理

#ifdef _DEBUG

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "TimelineConfig.h"

namespace ImGui {
namespace TimelineCore {

/// @brief ImGuiウィジェットの初期化データ
struct WidgetInitData {
    ImGuiWindow* window;
    ImGuiContext* context;
    ImGuiID id;
    ImRect frameBB;
    ImRect totalBB;
    float sliderWidth;
    bool isHovered;
    bool isValid;
};

/// @brief タイムラインウィジェットの状態
struct TimelineState {
    int draggedIndex;
    float draggedValue;
    float startDragValue;
    int popUpIndex;
};

/// @brief ImGuiストレージキーID
struct StorageKeys {
    ImGuiID draggedIndexId;
    ImGuiID draggedValueId;
    ImGuiID startDragValueId;
    ImGuiID popUpIndexId;
};

/// @brief ウィジェットの初期化処理
/// @param label ラベル
/// @return 初期化データ
inline WidgetInitData InitializeWidget(const std::string& label) {
    using namespace ::TimelineConfig;

    WidgetInitData data{};
    data.window  = GetCurrentWindow();
    data.context = GImGui;
    data.isValid = false;

    if (data.window->SkipItems) {
        return data;
    }

    const ImGuiStyle& style = data.context->Style;
    data.id                 = data.window->GetID(label.c_str());
    const float width       = CalcItemWidth();

    const ImVec2 labelSize = CalcTextSize(label.c_str(), NULL, true);
    data.frameBB           = ImRect(
        data.window->DC.CursorPos,
        ImVec2(data.window->DC.CursorPos.x + width, data.window->DC.CursorPos.y + FRAME_HEIGHT));
    data.totalBB = ImRect(
        data.frameBB.Min,
        ImVec2((labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f) + data.frameBB.Max.x, data.frameBB.Max.y));

    ItemSize(data.totalBB, style.FramePadding.y);
    if (!ItemAdd(data.totalBB, data.id, &data.frameBB)) {
        return data;
    }

    data.isHovered = ItemHoverable(data.frameBB, data.id, data.context->LastItemData.InFlags);

    // アクティブ状態の設定
    bool tempInputIsActive = TempInputIsActive(data.id);
    if (!tempInputIsActive) {
        const bool clicked    = data.isHovered && IsMouseClicked(0, data.id);
        const bool makeActive = (clicked || data.context->NavActivateId == data.id);
        if (makeActive && clicked)
            SetKeyOwner(ImGuiKey_MouseLeft, data.id);
        if (makeActive) {
            SetActiveID(data.id, data.window);
            SetFocusID(data.id, data.window);
            FocusWindow(data.window);
            data.context->ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }

    if (tempInputIsActive) {
        return data;
    }

    // フレームの描画
    const ImU32 frameCol = GetColorU32(
        data.context->ActiveId == data.id ? ImGuiCol_FrameBgActive
        : data.isHovered                  ? ImGuiCol_FrameBgHovered
                                          : ImGuiCol_FrameBg);
    RenderNavHighlight(data.frameBB, data.id);
    RenderFrame(data.frameBB.Min, data.frameBB.Max, frameCol, true, data.context->Style.FrameRounding);

    data.sliderWidth = data.frameBB.Max.x - data.frameBB.Min.x;
    data.isValid     = true;

    return data;
}

/// @brief ストレージキーを生成
/// @param baseId 基準ID
/// @return ストレージキー
inline StorageKeys CreateStorageKeys(ImGuiID baseId) {
    StorageKeys keys{};
    keys.draggedIndexId   = baseId + ImGui::GetID("draggedIndex");
    keys.draggedValueId   = baseId + ImGui::GetID("draggedValue");
    keys.startDragValueId = baseId + ImGui::GetID("startDragValue");
    keys.popUpIndexId     = baseId + ImGui::GetID("popUpIndex");
    return keys;
}

/// @brief 状態を読み込み
/// @param keys ストレージキー
/// @return タイムライン状態
inline TimelineState LoadState(const StorageKeys& keys) {
    ImGuiStorage* storage = ImGui::GetStateStorage();
    TimelineState state{};
    state.draggedIndex   = storage->GetInt(keys.draggedIndexId, -1);
    state.draggedValue   = storage->GetFloat(keys.draggedValueId, 0.0f);
    state.startDragValue = storage->GetFloat(keys.startDragValueId, 0.0f);
    state.popUpIndex     = storage->GetInt(keys.popUpIndexId, -1);
    return state;
}

/// @brief 状態を保存
/// @param keys ストレージキー
/// @param state タイムライン状態
inline void SaveState(const StorageKeys& keys, const TimelineState& state) {
    ImGuiStorage* storage = ImGui::GetStateStorage();
    storage->SetInt(keys.draggedIndexId, state.draggedIndex);
    storage->SetFloat(keys.draggedValueId, state.draggedValue);
    storage->SetFloat(keys.startDragValueId, state.startDragValue);
    storage->SetInt(keys.popUpIndexId, state.popUpIndex);
}

/// @brief ノードのボタン矩形を計算
/// @param frameBB フレーム境界ボックス
/// @param sliderWidth スライダー幅
/// @param time ノードの時間
/// @param duration アニメーション全体時間
/// @return ボタン矩形
inline ImRect CalculateNodeRect(const ImRect& frameBB, float sliderWidth, float time, float duration) {
    using namespace ::TimelineConfig;
    float t       = time / duration;
    float buttonX = frameBB.Min.x + t * sliderWidth;
    ImVec2 buttonPos(buttonX - BUTTON_SIZE * 0.5f, frameBB.Min.y);
    ImVec2 buttonEnd(buttonPos.x + BUTTON_SIZE, buttonPos.y + FRAME_HEIGHT);
    return ImRect(buttonPos, buttonEnd);
}

/// @brief ノードをデフォルトカラーで描画
/// @param drawList 描画リスト
/// @param rect ノード矩形
inline void DrawNode(ImDrawList* drawList, const ImRect& rect) {
    using namespace ::TimelineConfig;
    drawList->AddRectFilled(
        rect.Min, rect.Max,
        IM_COL32(NODE_COLOR_R, NODE_COLOR_G, NODE_COLOR_B, NODE_COLOR_A),
        GImGui->Style.FrameRounding);
}

/// @brief ノードをカスタムカラーで描画
/// @param drawList 描画リスト
/// @param rect ノード矩形
/// @param color カラー
inline void DrawColorNode(ImDrawList* drawList, const ImRect& rect, const ImVec4& color) {
    drawList->AddRectFilled(rect.Min, rect.Max, ImColor(color), GImGui->Style.FrameRounding);
}

/// @brief マウス位置から時間を計算
/// @param frameBB フレーム境界ボックス
/// @param duration アニメーション全体時間
/// @return 計算された時間
inline float CalculateTimeFromMouse(const ImRect& frameBB, float duration) {
    float sliderWidth = frameBB.Max.x - frameBB.Min.x;
    return ((GetMousePos().x - frameBB.Min.x) / sliderWidth) * duration;
}

/// @brief マウス位置から正規化された時間を計算 (0.0 - 1.0)
/// @param frameBB フレーム境界ボックス
/// @return 正規化された時間
inline float CalculateNormalizedTimeFromMouse(const ImRect& frameBB) {
    float sliderWidth = frameBB.Max.x - frameBB.Min.x;
    float t           = (GetMousePos().x - frameBB.Min.x) / sliderWidth;
    return ImClamp(t, 0.0f, 1.0f);
}

} // namespace TimelineCore
} // namespace ImGui

#endif // _DEBUG
