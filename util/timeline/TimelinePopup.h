#pragma once

/// @file TimelinePopup.h
/// @brief タイムラインエディタのポップアップ処理

#ifdef _DEBUG

#include "component/animation/AnimationData.h"
#include "TimelineConfig.h"
#include "TimelineCore.h"

// MyGui.h内のDragGuiCommandなどのテンプレートを使用
#include "myGui/MyGui.h"

#include <algorithm>
#include <memory>

namespace TimelinePopup {

/// @brief キーフレームをソートする
template <typename T>
inline void SortKeyFrames(OriGine::AnimationCurve<T>& keyFrames) {
    std::sort(
        keyFrames.begin(),
        keyFrames.end(),
        [](const auto& a, const auto& b) {
            return a.time < b.time;
        });
}

/// @brief ノード削除処理
/// @tparam T 値の型
/// @param keyFrames キーフレーム配列
/// @param popUpIndex 削除対象のインデックス
/// @param defaultValue デフォルト値
/// @return 成功時true
template <typename T>
inline bool HandleDeleteNode(
    OriGine::AnimationCurve<T>& keyFrames,
    int& popUpIndex,
    const T& defaultValue) {
    if (keyFrames.size() <= 1) {
        // 最後のキーフレームはデフォルト値にリセット
        OriGine::EditorController::GetInstance()->PushCommand(
            std::make_unique<SetterCommand<OriGine::KeyFrame<T>>>(
                &keyFrames[popUpIndex],
                OriGine::KeyFrame<T>(0.0f, defaultValue)));
        popUpIndex = -1;
        return true;
    }

    auto commandCombo = std::make_unique<CommandCombo>();
    commandCombo->AddCommand(
        std::make_shared<EraseElementCommand<OriGine::AnimationCurve<T>>>(
            &keyFrames, keyFrames.begin() + popUpIndex));
    OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
    popUpIndex = -1;
    return true;
}

/// @brief ノードコピー処理
/// @tparam T 値の型
/// @param keyFrames キーフレーム配列
/// @param popUpIndex コピー元のインデックス
/// @param popUpIndexId ストレージID
/// @return 成功時true
template <typename T>
inline bool HandleCopyNode(
    OriGine::AnimationCurve<T>& keyFrames,
    int popUpIndex,
    ImGuiID popUpIndexId) {
    using namespace TimelineConfig;

    auto commandCombo = std::make_unique<CommandCombo>();
    commandCombo->AddCommand(
        std::make_shared<AddElementCommand<OriGine::AnimationCurve<T>>>(
            &keyFrames, keyFrames[popUpIndex]));
    commandCombo->AddCommand(
        std::make_shared<SetterCommand<float>>(
            &keyFrames.back().time, keyFrames.back().time + COPY_TIME_OFFSET));

    commandCombo->SetFuncOnAfterCommand(
        [popUpIndexId, &keyFrames]() {
            ImGuiStorage* storage = ImGui::GetStateStorage();
            storage->SetInt(popUpIndexId, (int)keyFrames.size() - 1);
            SortKeyFrames(keyFrames);
        },
        false);

    commandCombo->SetFuncOnAfterUndoCommand(
        [popUpIndexId, popUpIndex, &keyFrames]() {
            ImGuiStorage* storage = ImGui::GetStateStorage();
            storage->SetInt(popUpIndexId, popUpIndex);
            SortKeyFrames(keyFrames);
        });

    OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
    return true;
}

/// @brief ノード追加処理
/// @tparam T 値の型
/// @param keyFrames キーフレーム配列
/// @param currentTime 追加位置の時間
/// @param popUpIndexId ストレージID
/// @param useCommand Undo/Redo対応コマンドを使用するか
/// @return 成功時true
template <typename T>
inline bool HandleAddNode(
    OriGine::AnimationCurve<T>& keyFrames,
    float currentTime,
    ImGuiID popUpIndexId,
    bool useCommand = false) {
    T newValue = OriGine::CalculateValue::Linear(keyFrames, currentTime);

    if (useCommand) {
        auto commandCombo = std::make_unique<CommandCombo>();
        commandCombo->AddCommand(
            std::make_shared<AddElementCommand<OriGine::AnimationCurve<T>>>(
                &keyFrames, OriGine::KeyFrame<T>(currentTime, newValue)));
        commandCombo->AddCommand(
            std::make_shared<SortCommand<OriGine::AnimationCurve<T>>>(
                &keyFrames, [](const auto& a, const auto& b) {
                    return a.time < b.time;
                }));
        commandCombo->SetFuncOnAfterCommand(
            [popUpIndexId, keyFrames]() {
                ImGuiStorage* storage = ImGui::GetStateStorage();
                storage->SetInt(popUpIndexId, (int)keyFrames.size() - 1);
            },
            true);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
    } else {
        keyFrames.push_back({currentTime, newValue});
        SortKeyFrames(keyFrames);
    }

    ImGui::CloseCurrentPopup();
    return true;
}

/// @brief タイム編集ドラッグUI
/// @tparam T 値の型
/// @param label ラベル
/// @param keyFrames キーフレーム配列
/// @param popUpIndex 編集対象のインデックス
template <typename T>
inline void DrawTimeEdit(
    const std::string& label,
    OriGine::AnimationCurve<T>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;

    ImGui::Text("Time");
    DragGuiCommand<float>(
        "##Time" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].time,
        DRAG_SPEED, {}, {}, "%.3f",
        [&keyFrames](float* /*val*/) {
            SortKeyFrames(keyFrames);
        });
}

/// @brief float値編集UI
inline void DrawValueEditFloat(
    const std::string& label,
    OriGine::AnimationCurve<float>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;
    ImGui::Text("Value");
    DragGuiCommand("##Value" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value, DRAG_SPEED);
}

/// @brief Vec2f値編集UI
inline void DrawValueEditVec2(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec2f>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;
    ImGui::Text("Value");
    DragGuiVectorCommand<2, float>(
        "##Value" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value, DRAG_SPEED);
}

/// @brief Vec3f値編集UI
inline void DrawValueEditVec3(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec3f>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;
    ImGui::Text("X:");
    DragGuiCommand<float>("##X" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::X], DRAG_SPEED);
    ImGui::Text("Y:");
    DragGuiCommand<float>("##Y" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::Y], DRAG_SPEED);
    ImGui::Text("Z:");
    DragGuiCommand<float>("##Z" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::Z], DRAG_SPEED);
}

/// @brief Vec4f値編集UI
inline void DrawValueEditVec4(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec4f>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;
    ImGui::Text("X:");
    DragGuiCommand<float>("##X" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::X], DRAG_SPEED);
    ImGui::Text("Y:");
    DragGuiCommand<float>("##Y" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::Y], DRAG_SPEED);
    ImGui::Text("Z:");
    DragGuiCommand<float>("##Z" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::Z], DRAG_SPEED);
    ImGui::Text("W:");
    DragGuiCommand<float>("##W" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value[OriGine::W], DRAG_SPEED);
}

/// @brief Quaternion値編集UI
inline void DrawValueEditQuaternion(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Quaternion>& keyFrames,
    int popUpIndex) {
    using namespace TimelineConfig;
    ImGui::Text("X:");
    DragGuiCommand<float>("##X" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value.v[OriGine::X], DRAG_SPEED);
    ImGui::Text("Y:");
    DragGuiCommand<float>("##Y" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value.v[OriGine::Y], DRAG_SPEED);
    ImGui::Text("Z:");
    DragGuiCommand<float>("##Z" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value.v[OriGine::Z], DRAG_SPEED);
    ImGui::Text("W:");
    DragGuiCommand<float>("##W" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value.v[OriGine::W], DRAG_SPEED);
    // Quaternionは正規化
    keyFrames[popUpIndex].value = keyFrames[popUpIndex].value.normalize();
}

/// @brief Color (Vec4f) 値編集UI
inline void DrawValueEditColor(
    const std::string& label,
    OriGine::AnimationCurve<OriGine::Vec4f>& keyFrames,
    int popUpIndex) {
    ColorEditGuiCommand<4>(
        "Color##" + label + std::to_string(popUpIndex),
        keyFrames[popUpIndex].value);
}

} // namespace TimelinePopup

#endif // _DEBUG
