/**
 * @file MyGuiCommand.h
 * @brief ImGuiのコマンド対応ラッパー関数（テンプレート実装）
 *
 * Undo/Redo機能を提供するコマンド付きGUIラッパー。
 * DragGuiCommand, SlideGuiCommand, InputGuiCommand, ColorEditGuiCommand等。
 */
#pragma once

#ifdef _DEBUG

/// stl
#include <functional>
#include <memory>
#include <string>

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"

/// myGui
#include "MyGuiBase.h"

//==============================================================================
// DragGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool DragGuiCommand(const std::string& label, T& value, float speed = 0.1f, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(T*)> afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = DragGui(label, value, speed, min, max, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value     = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, afterFunc));
        result = true;
    }

    return result;
}

//==============================================================================
// DragGuiVectorCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Drag を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <int N, typename T>
bool DragGuiVectorCommand(const std::string& label, OriGine::Vector<N, T>& value, float speed = 0.01f, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = DragVectorGui(label, value, speed, min, max, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = value;
        value                         = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&value, newValue, afterFunc));
        result = true;
    }

    return result;
}

//==============================================================================
// SlideGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool SlideGuiCommand(const std::string& label, T& value, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(T*)> afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = SlideGui(label, value, min, max, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value     = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// SlideVectorCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Slider を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T, int N>
bool SlideVectorCommand(const std::string& label, OriGine::Vector<N, T>& value, T min = T(), T max = T(), const char* format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = SlideVectorGui(label, value, min, max, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = value;
        value                         = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&value, newValue, afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// InputGuiCommand - スカラー値用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Input を型Tに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T>
bool InputGuiCommand(const std::string& label, T& value, const char* format = "%.3f", std::function<void(T*)> afterFunc = nullptr) {
    static GuiValuePool<T> valuePool;

    bool result = InputGui(label, value, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        T newValue = value;
        value     = valuePool.popValue(label);
        ::OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<T>>(&value, newValue, afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// InputVectorGuiCommand - N次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::Input を型TのN次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <typename T, int N>
bool InputVectorGuiCommand(const std::string& label, OriGine::Vector<N, T>& value, const char* format = "%.3f", std::function<void(OriGine::Vector<N, T>*)> afterFunc = nullptr) {
    static GuiValuePool<OriGine::Vector<N, T>> valuePool;

    bool result = InputVectorGui(label, value, format);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, T> newValue = value;
        value                         = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, T>>>(&value, newValue, afterFunc));
        return true;
    }

    return result;
}

//==============================================================================
// ColorEditGuiCommand - 3または4次元ベクトル用（Undo/Redo対応）
//==============================================================================

/// <summary>
/// ::ImGui::ColorEdit を型Nの3または4次元ベクトルに対応させたコマンド付き(Undo/Redoを可能にするための)ラッパー
/// </summary>
template <int N>
bool ColorEditGuiCommand(const std::string& label, OriGine::Vector<N, float>& value, ImGuiColorEditFlags colorEditFlags = 0, std::function<void(OriGine::Vector<N, float>*)> afterFunc = nullptr) {
    static_assert(N == 3 || N == 4, "ColorEditGuiCommand only supports 3 or 4 components (RGB or RGBA).");

    static GuiValuePool<OriGine::Vector<N, float>> valuePool;

    bool result = ColorEditGui<N>(label, value, colorEditFlags);

    if (::ImGui::IsItemActive()) {
        valuePool.SetValue(label, value);
    } else if (::ImGui::IsItemDeactivatedAfterEdit()) {
        OriGine::Vector<N, float> newValue = value;
        value                             = valuePool.popValue(label);
        OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<SetterCommand<OriGine::Vector<N, float>>>(&value, newValue, afterFunc));
        return true;
    }

    return result;
}

#endif // _DEBUG
